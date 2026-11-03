#include "FootballRefereeActor.h"

#include "FootballBallActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AFootballRefereeActor::AFootballRefereeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RefereeRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
    );
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );

    BodyProxy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RefereeBodyProxy"));
    BodyProxy->SetupAttachment(SceneRoot);
    if (CylinderAsset.Succeeded())
    {
        BodyProxy->SetStaticMesh(CylinderAsset.Object);
    }
    BodyProxy->SetRelativeLocation(FVector(0.0f, 0.0f, 92.0f));
    BodyProxy->SetRelativeScale3D(FVector(0.24f, 0.24f, 0.58f));
    BodyProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyProxy->SetCanEverAffectNavigation(false);

    HeadProxy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RefereeHeadProxy"));
    HeadProxy->SetupAttachment(SceneRoot);
    if (SphereAsset.Succeeded())
    {
        HeadProxy->SetStaticMesh(SphereAsset.Object);
    }
    HeadProxy->SetRelativeLocation(FVector(0.0f, 0.0f, 164.0f));
    HeadProxy->SetRelativeScale3D(FVector(0.20f, 0.20f, 0.20f));
    HeadProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeadProxy->SetCanEverAffectNavigation(false);
}

void AFootballRefereeActor::BeginPlay()
{
    Super::BeginPlay();
    FindTrackedBall();
}

void AFootballRefereeActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!IsValid(TrackedBall))
    {
        BallSearchAccumulator -= DeltaSeconds;
        if (BallSearchAccumulator <= 0.0f)
        {
            FindTrackedBall();
            BallSearchAccumulator = 0.75f;
        }
    }

    if (!bAutoTrackBall || !IsValid(TrackedBall))
    {
        return;
    }

    const FVector BallLocation = TrackedBall->GetActorLocation();
    FVector DesiredLocation = BallLocation + FollowOffsetCm;
    DesiredLocation.X = FMath::Clamp(DesiredLocation.X, -4900.0f, 4900.0f);
    DesiredLocation.Y = FMath::Clamp(DesiredLocation.Y, -3100.0f, 3100.0f);
    DesiredLocation.Z = GetActorLocation().Z;

    SetActorLocation(FMath::VInterpTo(
        GetActorLocation(), DesiredLocation, DeltaSeconds, FollowInterpSpeed
    ));

    FVector ToBall = BallLocation - GetActorLocation();
    ToBall.Z = 0.0f;
    if (!ToBall.IsNearlyZero())
    {
        const FRotator DesiredRotation(0.0f, ToBall.Rotation().Yaw, 0.0f);
        SetActorRotation(FMath::RInterpTo(
            GetActorRotation(), DesiredRotation, DeltaSeconds, FacingInterpSpeed
        ));
    }
}

void AFootballRefereeActor::FindTrackedBall()
{
    if (!GetWorld())
    {
        return;
    }

    for (TActorIterator<AFootballBallActor> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            TrackedBall = *It;
            return;
        }
    }
}
