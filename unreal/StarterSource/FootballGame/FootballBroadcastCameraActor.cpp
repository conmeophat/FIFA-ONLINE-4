#include "FootballBroadcastCameraActor.h"

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"

AFootballBroadcastCameraActor::AFootballBroadcastCameraActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    BroadcastCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BroadcastCamera"));
    BroadcastCamera->SetupAttachment(SceneRoot);
    BroadcastCamera->SetRelativeLocation(FVector::ZeroVector);
    BroadcastCamera->SetRelativeRotation(FRotator::ZeroRotator);
    BroadcastCamera->SetFieldOfView(FieldOfView);
    BroadcastCamera->bUsePawnControlRotation = false;
    BroadcastCamera->bAutoActivate = true;
}

void AFootballBroadcastCameraActor::BeginPlay()
{
    Super::BeginPlay();

    BroadcastCamera->SetFieldOfView(FieldOfView);

    if (bActivateAsPlayerViewOnBeginPlay)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PlayerController = World->GetFirstPlayerController())
            {
                PlayerController->SetViewTargetWithBlend(this, 0.75f);
                bHasActivatedPlayerView = true;
            }
        }
    }
}

void AFootballBroadcastCameraActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bActivateAsPlayerViewOnBeginPlay && !bHasActivatedPlayerView)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PlayerController = World->GetFirstPlayerController())
            {
                PlayerController->SetViewTargetWithBlend(this, 0.75f);
                bHasActivatedPlayerView = true;
            }
        }
    }

    const FVector FocusLocation = GetFocusLocation();
    const FVector DesiredLocation = GetDesiredCameraLocation(FocusLocation);

    const FVector NewLocation = FMath::VInterpTo(
        GetActorLocation(),
        DesiredLocation,
        DeltaSeconds,
        PositionInterpSpeed
    );

    SetActorLocation(NewLocation);

    const FVector DirectionToFocus = FocusLocation - NewLocation;
    if (!DirectionToFocus.IsNearlyZero())
    {
        const FRotator DesiredRotation = DirectionToFocus.Rotation();
        const FRotator NewRotation = FMath::RInterpTo(
            GetActorRotation(),
            DesiredRotation,
            DeltaSeconds,
            RotationInterpSpeed
        );

        SetActorRotation(NewRotation);
    }

    BroadcastCamera->SetFieldOfView(FieldOfView);
}

void AFootballBroadcastCameraActor::SetTrackedActor(AActor* NewTrackedActor)
{
    TrackedActor = NewTrackedActor;
}

void AFootballBroadcastCameraActor::SetManualFocusLocation(const FVector& NewFocusLocation)
{
    ManualFocusLocation = NewFocusLocation;
}

FVector AFootballBroadcastCameraActor::GetFocusLocation() const
{
    FVector Focus = IsValid(TrackedActor)
        ? TrackedActor->GetActorLocation()
        : ManualFocusLocation;

    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    Focus.X = FMath::Clamp(Focus.X, -HalfLength, HalfLength);
    Focus.Y = FMath::Clamp(Focus.Y, -HalfWidth, HalfWidth);
    Focus.Z += FocusHeightCm;

    return Focus;
}

FVector AFootballBroadcastCameraActor::GetDesiredCameraLocation(const FVector& FocusLocation) const
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    const float CameraX = FMath::Clamp(
        FocusLocation.X - CameraLeadCm,
        -HalfLength - 1200.0f,
        HalfLength + 1200.0f
    );

    // The camera stays outside the same touchline, like a conventional TV broadcast rig.
    const float CameraY = -(HalfWidth + DistanceFromSidelineCm);

    return FVector(CameraX, CameraY, CameraHeightCm);
}
