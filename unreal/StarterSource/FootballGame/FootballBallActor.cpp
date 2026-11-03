#include "FootballBallActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AFootballBallActor::AFootballBallActor()
{
    PrimaryActorTick.bCanEverTick = true;

    PhysicsSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BallPhysicsSphere"));
    SetRootComponent(PhysicsSphere);
    PhysicsSphere->InitSphereRadius(BallRadiusCm);
    PhysicsSphere->SetCollisionProfileName(TEXT("PhysicsActor"));
    PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicsSphere->SetGenerateOverlapEvents(true);
    PhysicsSphere->SetNotifyRigidBodyCollision(true);
    PhysicsSphere->SetLinearDamping(LinearDamping);
    PhysicsSphere->SetAngularDamping(AngularDamping);
    PhysicsSphere->SetMassOverrideInKg(NAME_None, BallMassKg, true);
    PhysicsSphere->SetCanEverAffectNavigation(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );
    BallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallVisual"));
    BallVisual->SetupAttachment(PhysicsSphere);
    if (SphereAsset.Succeeded())
    {
        BallVisual->SetStaticMesh(SphereAsset.Object);
    }
    BallVisual->SetRelativeScale3D(FVector(BallRadiusCm / 50.0f));
    BallVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BallVisual->SetGenerateOverlapEvents(false);
    BallVisual->SetCastShadow(true);

    PhysicsSphere->SetSimulatePhysics(true);
}

void AFootballBallActor::BeginPlay()
{
    Super::BeginPlay();

    BallPhysicalMaterial = NewObject<UPhysicalMaterial>(this, TEXT("FootballBallPhysicalMaterial"));
    if (BallPhysicalMaterial)
    {
        BallPhysicalMaterial->Friction = 0.55f;
        BallPhysicalMaterial->Restitution = 0.52f;
        PhysicsSphere->SetPhysMaterialOverride(BallPhysicalMaterial);
    }

    PreviousBallLocation = GetActorLocation();
}

void AFootballBallActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bWaitingForGoalReset)
    {
        CheckForGoal();
    }
    PreviousBallLocation = GetActorLocation();
}

bool AFootballBallActor::ApplyKick(
    const FVector& Direction,
    const float SpeedCmPerSec,
    const float LiftCmPerSec,
    const float SpinDegPerSec
)
{
    if (!PhysicsSphere || !PhysicsSphere->IsSimulatingPhysics())
    {
        return false;
    }

    const FVector HorizontalDirection = Direction.GetSafeNormal2D();
    if (HorizontalDirection.IsNearlyZero())
    {
        return false;
    }

    const FVector Impulse =
        HorizontalDirection * FMath::Max(0.0f, SpeedCmPerSec) +
        FVector::UpVector * FMath::Max(0.0f, LiftCmPerSec);
    PhysicsSphere->AddImpulse(Impulse, NAME_None, true);

    if (!FMath::IsNearlyZero(SpinDegPerSec))
    {
        const FVector CurrentAngularVelocity =
            PhysicsSphere->GetPhysicsAngularVelocityInDegrees(NAME_None);
        PhysicsSphere->SetPhysicsAngularVelocityInDegrees(
            CurrentAngularVelocity + FVector(0.0f, -SpinDegPerSec, 0.0f),
            false,
            NAME_None
        );
    }
    return true;
}

bool AFootballBallActor::ApplyKickFromFoot(
    const FVector& FootLocation,
    const FVector& Direction,
    const float SpeedCmPerSec,
    const float LiftCmPerSec,
    const float SpinDegPerSec
)
{
    if (FVector::Dist(FootLocation, GetActorLocation()) > KickContactRangeCm)
    {
        return false;
    }

    return ApplyKick(Direction, SpeedCmPerSec, LiftCmPerSec, SpinDegPerSec);
}

bool AFootballBallActor::ApplyFootTouch(
    const FVector& FootLocation,
    const FVector& PlayerForward,
    const FVector& PlayerVelocity
)
{
    if (!PhysicsSphere || !PhysicsSphere->IsSimulatingPhysics())
    {
        return false;
    }

    const FVector BallLocation = GetActorLocation();
    const float DistanceSquared = FVector::DistSquared(FootLocation, BallLocation);
    if (DistanceSquared > FMath::Square(FootTouchRangeCm))
    {
        return false;
    }

    FVector DesiredTarget = FootLocation + PlayerForward.GetSafeNormal2D() * 55.0f;
    DesiredTarget.Z = BallLocation.Z;

    FVector PositionError = DesiredTarget - BallLocation;
    PositionError.Z = 0.0f;

    const FVector CurrentVelocity = PhysicsSphere->GetPhysicsLinearVelocity(NAME_None);
    FVector DesiredVelocity = PlayerVelocity * 0.72f + PositionError * 1.9f;
    DesiredVelocity.Z = CurrentVelocity.Z;
    DesiredVelocity = DesiredVelocity.GetClampedToMaxSize2D(760.0f);

    const FVector AssistedVelocity = FMath::VInterpTo(
        CurrentVelocity,
        DesiredVelocity,
        GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f,
        5.5f
    );
    PhysicsSphere->SetPhysicsLinearVelocity(AssistedVelocity, false, NAME_None);
    return true;
}

void AFootballBallActor::ApplyDribbleControl(
    const FVector& TargetLocation,
    const FVector& PlayerVelocity,
    const float DeltaSeconds
)
{
    if (!PhysicsSphere || !PhysicsSphere->IsSimulatingPhysics() || DeltaSeconds <= 0.0f)
    {
        return;
    }

    const FVector CurrentVelocity = PhysicsSphere->GetPhysicsLinearVelocity(NAME_None);
    FVector PositionError = TargetLocation - GetActorLocation();
    PositionError.Z = 0.0f;

    FVector DesiredVelocity = PlayerVelocity * 0.82f + PositionError * 2.4f;
    DesiredVelocity.Z = CurrentVelocity.Z;
    DesiredVelocity = DesiredVelocity.GetClampedToMaxSize2D(850.0f);

    FVector AssistedVelocity = FMath::VInterpTo(
        CurrentVelocity, DesiredVelocity, DeltaSeconds, 4.0f
    );
    AssistedVelocity.Z = CurrentVelocity.Z;
    PhysicsSphere->SetPhysicsLinearVelocity(AssistedVelocity, false, NAME_None);
}

void AFootballBallActor::CheckForGoal()
{
    if (!PhysicsSphere || !PhysicsSphere->IsSimulatingPhysics())
    {
        return;
    }

    const FVector CurrentLocation = GetActorLocation();
    const FVector CurrentVelocity = PhysicsSphere->GetPhysicsLinearVelocity(NAME_None);
    const bool bCrossedPositiveGoal =
        PreviousBallLocation.X < PitchHalfLengthCm &&
        CurrentLocation.X >= PitchHalfLengthCm &&
        CurrentVelocity.X > 0.0f;
    const bool bCrossedNegativeGoal =
        PreviousBallLocation.X > -PitchHalfLengthCm &&
        CurrentLocation.X <= -PitchHalfLengthCm &&
        CurrentVelocity.X < 0.0f;

    if (!IsInsideGoalMouth(CurrentLocation))
    {
        return;
    }

    if (bCrossedPositiveGoal)
    {
        RegisterGoal(1);
    }
    else if (bCrossedNegativeGoal)
    {
        RegisterGoal(-1);
    }
}

bool AFootballBallActor::IsInsideGoalMouth(const FVector& BallLocation) const
{
    const float GoalHalfWidth = GoalWidthCm * 0.5f - BallRadiusCm;
    const float CrossbarClearance = GoalHeightCm - BallRadiusCm;

    return FMath::Abs(BallLocation.Y) <= GoalHalfWidth &&
        BallLocation.Z >= BallRadiusCm - 2.0f &&
        BallLocation.Z <= CrossbarClearance;
}

void AFootballBallActor::RegisterGoal(const int32 GoalSide)
{
    if (bWaitingForGoalReset)
    {
        return;
    }

    if (GoalSide > 0)
    {
        ++PositiveXScore;
    }
    else
    {
        ++NegativeXScore;
    }

    OnGoalScored.Broadcast(GoalSide, PositiveXScore, NegativeXScore);
    bWaitingForGoalReset = true;
    PhysicsSphere->SetSimulatePhysics(false);
    PhysicsSphere->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
    PhysicsSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false, NAME_None);
    SetActorLocation(
        FVector(0.0f, 0.0f, BallRadiusCm + 1.0f),
        false,
        nullptr,
        ETeleportType::TeleportPhysics
    );

    GetWorldTimerManager().SetTimer(
        GoalResetTimer,
        this,
        &AFootballBallActor::ResetAfterGoal,
        1.5f,
        false
    );
}

void AFootballBallActor::ResetAfterGoal()
{
    if (PhysicsSphere)
    {
        PhysicsSphere->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
        PhysicsSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false, NAME_None);
        PhysicsSphere->SetSimulatePhysics(true);
    }

    PreviousBallLocation = GetActorLocation();
    bWaitingForGoalReset = false;
}
