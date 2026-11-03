#include "FootballPlayerCharacter.h"

#include "FootballBallActor.h"
#include "FootballMatchDirector.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AFootballPlayerCharacter::AFootballPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(38.0f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = WalkSpeedCmPerSec;
    Movement->MaxAcceleration = AccelerationCmPerSec2;
    Movement->BrakingDecelerationWalking = BrakingDecelerationCmPerSec2;
    Movement->GroundFriction = 8.0f;
    Movement->BrakingFrictionFactor = 2.0f;
    Movement->bUseSeparateBrakingFriction = true;
    Movement->bOrientRotationToMovement = false;
    Movement->RotationRate = FRotator(0.0f, 620.0f, 0.0f);
    Movement->JumpZVelocity = 420.0f;
    Movement->AirControl = 0.25f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("PlayerCameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraArmLengthCm;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 85.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 9.0f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 12.0f;
    CameraBoom->bDoCollisionTest = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerFollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->SetFieldOfView(80.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
        TEXT("/Engine/BasicShapes/Cube.Cube")
    );
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
    );

    if (CubeAsset.Succeeded())
    {
        ProxyCubeMesh = CubeAsset.Object;
    }
    if (SphereAsset.Succeeded())
    {
        ProxySphereMesh = SphereAsset.Object;
    }
    if (CylinderAsset.Succeeded())
    {
        ProxyCylinderMesh = CylinderAsset.Object;
    }

    auto CreateProxy = [this](
        const FName ComponentName,
        UStaticMesh* Mesh,
        const FVector& Location,
        const FVector& Scale,
        const FRotator& Rotation
    ) -> UStaticMeshComponent*
    {
        UStaticMeshComponent* Component =
            CreateDefaultSubobject<UStaticMeshComponent>(ComponentName);
        Component->SetupAttachment(RootComponent);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeLocation(Location);
        Component->SetRelativeScale3D(Scale);
        Component->SetRelativeRotation(Rotation);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCastShadow(true);
        return Component;
    };

    // Temporary, engine-built proxy; use a licensed Skeletal Mesh for production.
    TorsoProxy = CreateProxy(
        TEXT("Proxy_Torso"), ProxyCubeMesh,
        FVector(0.0f, 0.0f, 112.0f), FVector(0.46f, 0.28f, 0.68f),
        FRotator::ZeroRotator
    );
    ShortsProxy = CreateProxy(
        TEXT("Proxy_Shorts"), ProxyCubeMesh,
        FVector(0.0f, 0.0f, 68.0f), FVector(0.43f, 0.30f, 0.25f),
        FRotator::ZeroRotator
    );
    HeadProxy = CreateProxy(
        TEXT("Proxy_Head"), ProxySphereMesh,
        FVector(0.0f, 0.0f, 160.0f), FVector(0.27f, 0.27f, 0.29f),
        FRotator::ZeroRotator
    );
    LeftArmProxy = CreateProxy(
        TEXT("Proxy_LeftArm"), ProxyCylinderMesh,
        FVector(0.0f, -39.0f, 108.0f), FVector(0.14f, 0.14f, 0.52f),
        FRotator(0.0f, 0.0f, -8.0f)
    );
    RightArmProxy = CreateProxy(
        TEXT("Proxy_RightArm"), ProxyCylinderMesh,
        FVector(0.0f, 39.0f, 108.0f), FVector(0.14f, 0.14f, 0.52f),
        FRotator(0.0f, 0.0f, 8.0f)
    );
    LeftLegProxy = CreateProxy(
        TEXT("Proxy_LeftLeg"), ProxyCylinderMesh,
        FVector(0.0f, -15.0f, 35.0f), FVector(0.16f, 0.16f, 0.48f),
        FRotator::ZeroRotator
    );
    RightLegProxy = CreateProxy(
        TEXT("Proxy_RightLeg"), ProxyCylinderMesh,
        FVector(0.0f, 15.0f, 35.0f), FVector(0.16f, 0.16f, 0.48f),
        FRotator::ZeroRotator
    );

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetHiddenInGame(true);
}

void AFootballPlayerCharacter::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    BallControlLockout = FMath::Max(0.0f, BallControlLockout - DeltaSeconds);

    if (!IsValid(ControlledBall) && BallControlLockout <= 0.0f)
    {
        BallSearchAccumulator -= DeltaSeconds;
        if (BallSearchAccumulator <= 0.0f)
        {
            RefreshNearbyBall();
            BallSearchAccumulator = 0.20f;
        }
    }

    if (IsValid(ControlledBall))
    {
        UpdateBallControl(DeltaSeconds);
    }
    else
    {
        bHasBallControl = false;
        bIsDribbling = false;
    }

    ContestStunRemaining = FMath::Max(0.0f, ContestStunRemaining - DeltaSeconds);
    bIsContestStunned = ContestStunRemaining > 0.0f;
    ContestReactionRemaining = FMath::Max(0.0f, ContestReactionRemaining - DeltaSeconds);
    bIsInShoulderDuel = ContestReactionRemaining > 0.0f;
    ContestReactionStrength = FMath::FInterpTo(
        ContestReactionStrength, bIsInShoulderDuel ? ContestReactionStrength : 0.0f,
        DeltaSeconds, 5.0f
    );
    UpdatePlayerContests(DeltaSeconds);

    UpdateBalance(DeltaSeconds);
    UpdateFootballFacing(DeltaSeconds);
    UpdateDynamicMovementTuning();
    PreviousVelocity = GetVelocity();
}

void AFootballPlayerCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    CameraBoom->TargetArmLength = CameraArmLengthCm;
    UpdateSprintSpeed();
    RefreshAppearance();
}

void AFootballPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        FRotator ControlRotation = PlayerController->GetControlRotation();
        if (FMath::IsNearlyZero(ControlRotation.Pitch, 2.0f))
        {
            ControlRotation.Pitch = InitialCameraPitch;
            PlayerController->SetControlRotation(ControlRotation);
        }
    }

    PreviousVelocity = GetVelocity();
    PreviousVelocity = GetVelocity();
    RefreshAppearance();
}

void AFootballPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis(
        TEXT("MoveForward"), this, &AFootballPlayerCharacter::MoveForward
    );
    PlayerInputComponent->BindAxis(
        TEXT("MoveRight"), this, &AFootballPlayerCharacter::MoveRight
    );
    PlayerInputComponent->BindAxis(
        TEXT("Turn"), this, &APawn::AddControllerYawInput
    );
    PlayerInputComponent->BindAxis(
        TEXT("LookUp"), this, &APawn::AddControllerPitchInput
    );

    PlayerInputComponent->BindAction(
        TEXT("Sprint"), IE_Pressed, this, &AFootballPlayerCharacter::StartSprint
    );
    PlayerInputComponent->BindAction(
        TEXT("Sprint"), IE_Released, this, &AFootballPlayerCharacter::StopSprint
    );
    PlayerInputComponent->BindAction(
        TEXT("Jump"), IE_Pressed, this, &AFootballPlayerCharacter::StartJump
    );
    PlayerInputComponent->BindAction(
        TEXT("Jump"), IE_Released, this, &AFootballPlayerCharacter::StopJump
    );
    PlayerInputComponent->BindAction(
        TEXT("Pass"), IE_Pressed, this, &AFootballPlayerCharacter::StartPass
    );
    PlayerInputComponent->BindAction(
        TEXT("ThroughBall"), IE_Pressed, this, &AFootballPlayerCharacter::StartThroughBall
    );
    PlayerInputComponent->BindAction(
        TEXT("Shoot"), IE_Pressed, this, &AFootballPlayerCharacter::StartShot
    );
    PlayerInputComponent->BindAction(
        TEXT("Brace"), IE_Pressed, this, &AFootballPlayerCharacter::StartBrace
    );
    PlayerInputComponent->BindAction(
        TEXT("Brace"), IE_Released, this, &AFootballPlayerCharacter::StopBrace
    );
}

void AFootballPlayerCharacter::MoveForward(const float Value)
{
    if (Controller == nullptr || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    AddMovementInput(ForwardDirection, Value);
}

void AFootballPlayerCharacter::MoveRight(const float Value)
{
    if (Controller == nullptr || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(RightDirection, Value);
}

void AFootballPlayerCharacter::StartSprint()
{
    bIsSprinting = true;
    UpdateSprintSpeed();
}

void AFootballPlayerCharacter::StopSprint()
{
    bIsSprinting = false;
    UpdateSprintSpeed();
}

void AFootballPlayerCharacter::StartJump()
{
    Jump();
}

void AFootballPlayerCharacter::StopJump()
{
    StopJumping();
}

void AFootballPlayerCharacter::StartBrace()
{
    bIsBracing = true;
    UpdateDynamicMovementTuning();
}

void AFootballPlayerCharacter::StopBrace()
{
    bIsBracing = false;
    UpdateDynamicMovementTuning();
}

bool AFootballPlayerCharacter::TryExecuteAITacticalKick(
    const EFootballTacticalAction Action,
    const FVector& TargetLocation
)
{
    if (Action == EFootballTacticalAction::None ||
        bIsContestStunned ||
        PendingBallAction != EFootballBallAction::None)
    {
        return false;
    }

    if (!IsValid(ControlledBall))
    {
        RefreshNearbyBall();
    }
    if (!IsValid(ControlledBall) ||
        FVector::Dist2D(GetActorLocation(), ControlledBall->GetActorLocation()) > 320.0f)
    {
        return false;
    }

    const FVector BallLocation = ControlledBall->GetActorLocation();
    const FVector KickDirection = (TargetLocation - BallLocation).GetSafeNormal2D();
    if (KickDirection.IsNearlyZero())
    {
        return false;
    }

    float SpeedCmPerSec = 0.0f;
    float LiftCmPerSec = 0.0f;
    float SpinDegPerSec = 0.0f;
    EFootballBallAction VisualAction = EFootballBallAction::ShortPass;

    switch (Action)
    {
    case EFootballTacticalAction::ShortPass:
        SpeedCmPerSec = 1480.0f;
        LiftCmPerSec = 18.0f;
        SpinDegPerSec = 65.0f;
        VisualAction = EFootballBallAction::ShortPass;
        break;
    case EFootballTacticalAction::ThroughBall:
        SpeedCmPerSec = 1900.0f;
        LiftCmPerSec = 95.0f;
        SpinDegPerSec = 120.0f;
        VisualAction = EFootballBallAction::ThroughBall;
        break;
    case EFootballTacticalAction::Cross:
    case EFootballTacticalAction::CornerKick:
        SpeedCmPerSec = 1780.0f;
        LiftCmPerSec = 520.0f;
        SpinDegPerSec = 170.0f;
        VisualAction = EFootballBallAction::ThroughBall;
        break;
    case EFootballTacticalAction::Clear:
        SpeedCmPerSec = 2150.0f;
        LiftCmPerSec = 240.0f;
        SpinDegPerSec = 150.0f;
        VisualAction = EFootballBallAction::Shot;
        break;
    case EFootballTacticalAction::Shot:
        SpeedCmPerSec = 2550.0f;
        LiftCmPerSec = 350.0f;
        SpinDegPerSec = 230.0f;
        VisualAction = EFootballBallAction::Shot;
        break;
    case EFootballTacticalAction::ThrowIn:
        SpeedCmPerSec = 1080.0f; LiftCmPerSec = 500.0f; SpinDegPerSec = 15.0f;
        VisualAction = EFootballBallAction::ThroughBall;
        break;
    case EFootballTacticalAction::GoalKick:
        SpeedCmPerSec = 2450.0f; LiftCmPerSec = 650.0f; SpinDegPerSec = 110.0f;
        VisualAction = EFootballBallAction::ThroughBall;
        break;
    case EFootballTacticalAction::FreeKick:
        SpeedCmPerSec = 1850.0f; LiftCmPerSec = 125.0f; SpinDegPerSec = 130.0f;
        VisualAction = EFootballBallAction::ThroughBall;
        break;
    case EFootballTacticalAction::PenaltyKick:
        SpeedCmPerSec = 2750.0f; LiftCmPerSec = 95.0f; SpinDegPerSec = 210.0f;
        VisualAction = EFootballBallAction::Shot;
        break;
    default:
        return false;
    }

    const float BalancePowerScale = FMath::Lerp(0.72f, 1.0f, Balance);
    SpeedCmPerSec *= BalancePowerScale;
    LiftCmPerSec *= BalancePowerScale;

    const EFootballContactFoot ContactFoot = ChooseNearestFoot();
    const FVector FootLocation = GetBallContactLocation(ContactFoot);
    const bool bKicked = ControlledBall->ApplyKickFromFoot(
        FootLocation,
        KickDirection,
        SpeedCmPerSec,
        LiftCmPerSec,
        SpinDegPerSec
    );

    if (!bKicked)
    {
        return false;
    }

    ControlledBall->RegisterLastTouch(TeamId, this);
    for (TActorIterator<AFootballMatchDirector> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            It->NotifyBallPlayed(this, Action);
            break;
        }
    }

    GetWorldTimerManager().ClearTimer(PendingContactFallbackTimer);
    GetWorldTimerManager().ClearTimer(BallActionStateTimer);
    PendingBallAction = EFootballBallAction::None;
    ActiveBallAction = VisualAction;
    ActiveContactFoot = ContactFoot;
    GetWorldTimerManager().SetTimer(
        BallActionStateTimer,
        this,
        &AFootballPlayerCharacter::EndBallActionState,
        0.62f,
        false
    );

    ControlledBall = nullptr;
    bHasBallControl = false;
    bIsDribbling = false;
    BallControlLockout = 0.72f;
    BallSearchAccumulator = 0.72f;
    return true;
}

void AFootballPlayerCharacter::ApplyContestImpact(
    const FVector& ImpactDirection,
    const float ImpulseCmPerSec,
    const float BalanceDamage
)
{
    const FVector SafeDirection = ImpactDirection.GetSafeNormal2D();
    if (SafeDirection.IsNearlyZero())
    {
        return;
    }

    ContestReactionRemaining = FMath::Max(ContestReactionRemaining, 0.34f);
    bIsInShoulderDuel = true;
    ContestReactionStrength = FMath::Max(
        ContestReactionStrength,
        FMath::Clamp(ImpulseCmPerSec / 260.0f, 0.15f, 1.0f)
    );

    const float Resistance = bIsBracing
        ? FMath::Clamp(BraceContestResistance, 0.0f, 0.9f)
        : 0.0f;
    const float RemainingImpact = 1.0f - Resistance;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        const float SafeImpulse = FMath::Clamp(ImpulseCmPerSec, 0.0f, 1100.0f);
        Movement->AddImpulse(SafeDirection * SafeImpulse * RemainingImpact, true);
    }

    const float SafeDamage = FMath::Clamp(BalanceDamage, 0.0f, 0.8f);
    Balance = FMath::Clamp(
        Balance - SafeDamage * RemainingImpact,
        0.0f,
        1.0f
    );

    ContestStunRemaining = FMath::Max(
        ContestStunRemaining,
        0.12f + FMath::Clamp(ImpulseCmPerSec, 0.0f, 1100.0f) / 3200.0f
    );
    bIsContestStunned = true;
    bIsOffBalance = Balance < 0.42f;
    UpdateDynamicMovementTuning();
}

void AFootballPlayerCharacter::UpdatePlayerContests(const float DeltaSeconds)
{
    if (!bEnableAutomaticPlayerContests || !GetWorld())
    {
        return;
    }

    PlayerContestCooldown = FMath::Max(0.0f, PlayerContestCooldown - DeltaSeconds);
    PlayerContestScanAccumulator -= DeltaSeconds;
    if (PlayerContestScanAccumulator > 0.0f || PlayerContestCooldown > 0.0f)
    {
        return;
    }
    PlayerContestScanAccumulator = 0.16f;

    for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
    {
        AFootballPlayerCharacter* Opponent = *It;
        if (!IsValid(Opponent) || Opponent == this || Opponent->TeamId == TeamId)
        {
            continue;
        }

        // One deterministic initiator per pair prevents duplicate impulses.
        if (GetUniqueID() > Opponent->GetUniqueID())
        {
            continue;
        }

        const FVector Separation = GetActorLocation() - Opponent->GetActorLocation();
        const float Distance = Separation.Size2D();
        if (Distance > FMath::Max(70.0f, ContestContactRadiusCm))
        {
            continue;
        }

        const FVector RelativeVelocity = GetVelocity() - Opponent->GetVelocity();
        const float RelativeSpeed = RelativeVelocity.Size2D();
        const bool bBallCarrierInvolved = bHasBallControl || Opponent->bHasBallControl;
        if (RelativeSpeed < 100.0f && !bIsBracing && !Opponent->bIsBracing && !bBallCarrierInvolved)
        {
            continue;
        }

        FVector PushDirection = Separation.GetSafeNormal2D();
        if (PushDirection.IsNearlyZero())
        {
            PushDirection = GetActorRightVector();
        }

        const float Impact = FMath::Clamp(65.0f + RelativeSpeed * 0.24f +
            (bBallCarrierInvolved ? 48.0f : 0.0f), 65.0f, 235.0f);
        const float Damage = bBallCarrierInvolved ? 0.085f : 0.055f;
        ApplyContestImpact(PushDirection, Impact, Damage);
        Opponent->ApplyContestImpact(-PushDirection, Impact, Damage);
        PlayerContestCooldown = 0.82f;
        break;
    }
}

void AFootballPlayerCharacter::UpdateBalance(const float DeltaSeconds)
{
    if (DeltaSeconds <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector CurrentVelocity = GetVelocity();
    FVector VelocityDelta = (CurrentVelocity - PreviousVelocity) / DeltaSeconds;
    VelocityDelta.Z = 0.0f;
    const FVector VelocityDirection = CurrentVelocity.GetSafeNormal2D();
    const FVector LateralAcceleration = VelocityDirection.IsNearlyZero()
        ? VelocityDelta
        : VelocityDelta - VelocityDirection * FVector::DotProduct(
            VelocityDelta,
            VelocityDirection
        );

    const float CurrentSpeed = CurrentVelocity.Size2D();
    const float ReferenceSpeed = FMath::Max(
        WalkSpeedCmPerSec,
        WalkSpeedCmPerSec * FMath::Max(1.0f, SprintSpeedMultiplier)
    );
    const float SpeedFactor = FMath::Clamp(CurrentSpeed / ReferenceSpeed, 0.0f, 1.0f);
    const float LateralRatio = FMath::Clamp(
        LateralAcceleration.Size() / FMath::Max(1.0f, LateralAccelerationBalanceThreshold),
        0.0f,
        2.5f
    );

    float BalanceLoss = LateralRatio * BalanceLossPerSecondAtThreshold *
        SpeedFactor * DeltaSeconds;

    if (bIsSprinting)
    {
        BalanceLoss *= 1.18f;
    }
    if (bIsBracing)
    {
        BalanceLoss *= 0.45f;
    }
    if (bHasBallControl)
    {
        BalanceLoss *= 0.92f;
    }

    Balance = FMath::Clamp(Balance - BalanceLoss, 0.0f, 1.0f);

    const float RecoveryRate = BalanceRecoveryPerSecond *
        (bIsBracing ? 1.55f : 1.0f);
    if (BalanceLoss <= KINDA_SMALL_NUMBER && !bIsContestStunned)
    {
        Balance = FMath::Clamp(Balance + RecoveryRate * DeltaSeconds, 0.0f, 1.0f);
    }

    const FVector RequestedDirection = GetLastMovementInputVector().GetSafeNormal2D();
    const float Alignment = (
        VelocityDirection.IsNearlyZero() || RequestedDirection.IsNearlyZero()
    )
        ? 1.0f
        : FVector::DotProduct(VelocityDirection, RequestedDirection);

    bIsChangingDirection = CurrentSpeed > 170.0f &&
        !RequestedDirection.IsNearlyZero() &&
        Alignment < 0.72f;

    if (bIsChangingDirection && bIsSprinting && !bIsBracing)
    {
        const float ExtraTurnStress = FMath::Clamp(
            (0.72f - Alignment) * 0.35f,
            0.0f,
            0.35f
        );
        Balance = FMath::Clamp(
            Balance - ExtraTurnStress * DeltaSeconds,
            0.0f,
            1.0f
        );
    }

    bIsOffBalance = Balance < 0.38f || bIsContestStunned;

    if (bIsContestStunned)
    {
        BalanceState = EFootballBalanceState::ContestStunned;
    }
    else if (bIsBracing)
    {
        BalanceState = EFootballBalanceState::Bracing;
    }
    else if (bIsOffBalance)
    {
        BalanceState = EFootballBalanceState::OffBalance;
    }
    else if (Balance < 0.96f)
    {
        BalanceState = EFootballBalanceState::Recovering;
    }
    else
    {
        BalanceState = EFootballBalanceState::Stable;
    }
}

void AFootballPlayerCharacter::UpdateFootballFacing(const float DeltaSeconds)
{
    if (DeltaSeconds <= KINDA_SMALL_NUMBER || bIsContestStunned)
    {
        return;
    }

    const FVector RequestedDirection = GetLastMovementInputVector().GetSafeNormal2D();
    const float CurrentSpeed = GetVelocity().Size2D();
    FVector DesiredFacing = FVector::ZeroVector;

    if (!RequestedDirection.IsNearlyZero())
    {
        DesiredFacing = RequestedDirection;
    }
    else if (IsValid(ControlledBall) &&
        FVector::DistSquared2D(GetActorLocation(), ControlledBall->GetActorLocation()) <=
            FMath::Square(BallFacingRangeCm) &&
        CurrentSpeed < 260.0f)
    {
        DesiredFacing = (
            ControlledBall->GetActorLocation() - GetActorLocation()
        ).GetSafeNormal2D();
    }
    else if (CurrentSpeed > 45.0f)
    {
        DesiredFacing = GetVelocity().GetSafeNormal2D();
    }

    if (DesiredFacing.IsNearlyZero())
    {
        return;
    }

    const float TurnRate = bIsSprinting
        ? SprintTurnRateDegreesPerSec
        : WalkingTurnRateDegreesPerSec;
    const float BalanceTurnScale = FMath::Lerp(0.55f, 1.0f, Balance);
    const float BraceTurnScale = bIsBracing ? 0.72f : 1.0f;
    const float TargetYaw = DesiredFacing.Rotation().Yaw;
    const float NewYaw = FMath::FixedTurn(
        GetActorRotation().Yaw,
        TargetYaw,
        TurnRate * BalanceTurnScale * BraceTurnScale * DeltaSeconds
    );

    SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));
}

void AFootballPlayerCharacter::UpdateDynamicMovementTuning()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement)
    {
        return;
    }

    const float BaseSpeed = bIsSprinting
        ? WalkSpeedCmPerSec * FMath::Max(1.0f, SprintSpeedMultiplier)
        : WalkSpeedCmPerSec;

    float TurnSpeedScale = 1.0f;
    const FVector VelocityDirection = GetVelocity().GetSafeNormal2D();
    const FVector RequestedDirection = GetLastMovementInputVector().GetSafeNormal2D();

    if (!VelocityDirection.IsNearlyZero() && !RequestedDirection.IsNearlyZero())
    {
        const float Alignment = FVector::DotProduct(
            VelocityDirection,
            RequestedDirection
        );
        const float TurnSeverity = FMath::Clamp(
            (1.0f - Alignment) * 0.5f,
            0.0f,
            1.0f
        );
        TurnSpeedScale = 1.0f -
            FMath::Clamp(DirectionChangeSpeedLoss, 0.0f, 0.8f) * TurnSeverity;
    }

    const float BalanceScale = FMath::Lerp(
        FMath::Clamp(OffBalanceSpeedMultiplier, 0.1f, 1.0f),
        1.0f,
        Balance
    );
    const float BraceScale = bIsBracing
        ? FMath::Clamp(BraceMovementMultiplier, 0.1f, 1.0f)
        : 1.0f;
    const float ContestScale = bIsContestStunned ? 0.34f : 1.0f;

    Movement->MaxWalkSpeed = BaseSpeed * TurnSpeedScale *
        BalanceScale * BraceScale * ContestScale;
    Movement->MaxAcceleration = AccelerationCmPerSec2 *
        FMath::Lerp(0.55f, 1.0f, Balance) *
        (bIsBracing ? 0.72f : 1.0f);
    Movement->BrakingDecelerationWalking = BrakingDecelerationCmPerSec2;
    Movement->GroundFriction = bIsBracing ? 11.0f : 8.0f;
    Movement->BrakingFrictionFactor = bIsBracing ? 2.7f : 2.0f;
}

void AFootballPlayerCharacter::UpdateSprintSpeed()
{
    UpdateDynamicMovementTuning();
}

void AFootballPlayerCharacter::RefreshAppearance()
{
    const bool bHasSkeletalMesh = IsValid(PlayerSkeletalMesh);

    if (bHasSkeletalMesh)
    {
        if (GetMesh()->GetSkeletalMeshAsset() != PlayerSkeletalMesh)
        {
            GetMesh()->SetSkeletalMesh(PlayerSkeletalMesh, true);
        }

        GetMesh()->SetRelativeLocation(SkeletalMeshRelativeLocation);
        GetMesh()->SetRelativeRotation(SkeletalMeshRelativeRotation);
        GetMesh()->SetVisibility(true, true);
        GetMesh()->SetHiddenInGame(false);

        if (PlayerAnimationBlueprint)
        {
            GetMesh()->SetAnimInstanceClass(PlayerAnimationBlueprint);
        }
    }
    else
    {
        GetMesh()->SetVisibility(false, true);
        GetMesh()->SetHiddenInGame(true);
    }

    const bool bShowProxy = !bHasSkeletalMesh;
    const TArray<UStaticMeshComponent*> ProxyParts = {
        TorsoProxy, HeadProxy, ShortsProxy,
        LeftArmProxy, RightArmProxy, LeftLegProxy, RightLegProxy
    };

    for (UStaticMeshComponent* Part : ProxyParts)
    {
        if (Part)
        {
            Part->SetVisibility(bShowProxy, true);
            Part->SetHiddenInGame(!bShowProxy);
        }
    }

    if (TorsoProxy) TorsoProxy->SetMaterial(0, ProxyKitMaterial);
    if (LeftArmProxy) LeftArmProxy->SetMaterial(0, ProxyKitMaterial);
    if (RightArmProxy) RightArmProxy->SetMaterial(0, ProxyKitMaterial);
    if (HeadProxy) HeadProxy->SetMaterial(0, ProxyKitMaterial);
    if (ShortsProxy) ShortsProxy->SetMaterial(0, ProxyShortsMaterial);
    if (LeftLegProxy) LeftLegProxy->SetMaterial(0, ProxyBootsMaterial);
    if (RightLegProxy) RightLegProxy->SetMaterial(0, ProxyBootsMaterial);
}

float AFootballPlayerCharacter::GetHorizontalSpeed() const
{
    return GetVelocity().Size2D();
}

float AFootballPlayerCharacter::GetMovementDirection() const
{
    const FVector Velocity = GetVelocity();
    if (Velocity.SizeSquared2D() < 1.0f)
    {
        return 0.0f;
    }

    const float MovementYaw = Velocity.ToOrientationRotator().Yaw;
    return FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, MovementYaw);
}

FVector AFootballPlayerCharacter::GetBallContactLocation(
    const EFootballContactFoot ContactFoot
) const
{
    const bool bUseLeftFoot = ContactFoot == EFootballContactFoot::Left;
    const FName SocketName = bUseLeftFoot
        ? LeftFootContactSocketName
        : RightFootContactSocketName;

    if (GetMesh() && GetMesh()->DoesSocketExist(SocketName))
    {
        return GetMesh()->GetSocketLocation(SocketName);
    }

    // Fallback is only for the primitive development mannequin / unconfigured rigs.
    const FVector SideOffset = GetActorRightVector() * (bUseLeftFoot ? -18.0f : 18.0f);
    return GetActorLocation() + GetActorForwardVector() * 70.0f +
        SideOffset + FVector(0.0f, 0.0f, -78.0f);
}

EFootballContactFoot AFootballPlayerCharacter::ChooseNearestFoot() const
{
    if (!IsValid(ControlledBall))
    {
        return EFootballContactFoot::Right;
    }

    const FVector BallLocation = ControlledBall->GetActorLocation();
    const float LeftDistance = FVector::DistSquared(
        BallLocation, GetBallContactLocation(EFootballContactFoot::Left)
    );
    const float RightDistance = FVector::DistSquared(
        BallLocation, GetBallContactLocation(EFootballContactFoot::Right)
    );
    return LeftDistance <= RightDistance
        ? EFootballContactFoot::Left
        : EFootballContactFoot::Right;
}

void AFootballPlayerCharacter::RefreshNearbyBall()
{
    if (!GetWorld())
    {
        return;
    }

    const float SearchRangeCm = FMath::Max(300.0f, BallFacingRangeCm);
    float BestDistanceSquared = FMath::Square(SearchRangeCm);
    AFootballBallActor* BestBall = nullptr;

    for (TActorIterator<AFootballBallActor> It(GetWorld()); It; ++It)
    {
        AFootballBallActor* Candidate = *It;
        if (!IsValid(Candidate))
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(
            GetActorLocation(), Candidate->GetActorLocation()
        );
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestBall = Candidate;
        }
    }

    ControlledBall = BestBall;
}

void AFootballPlayerCharacter::UpdateBallControl(const float DeltaSeconds)
{
    if (!IsValid(ControlledBall) || GetCharacterMovement()->IsFalling() ||
        PendingBallAction != EFootballBallAction::None)
    {
        bHasBallControl = false;
        bIsDribbling = false;
        return;
    }

    const FVector ToBall = ControlledBall->GetActorLocation() - GetActorLocation();
    if (ToBall.SizeSquared2D() > FMath::Square(BallFacingRangeCm))
    {
        ControlledBall = nullptr;
        bHasBallControl = false;
        bIsDribbling = false;
        return;
    }

    const bool bBallWithinFootControlRange =
        ToBall.SizeSquared2D() <= FMath::Square(300.0f);
    const bool bBallInFront =
        FVector::DotProduct(GetActorForwardVector(), ToBall.GetSafeNormal2D()) >= -0.35f;

    if (!bBallWithinFootControlRange || !bBallInFront)
    {
        // Keep tracking the ball at a wider radius so the body can turn toward
        // it, but do not apply close-control touches until it is in front and near.
        bHasBallControl = false;
        bIsDribbling = false;
        return;
    }

    bHasBallControl = true;
    bIsDribbling = GetHorizontalSpeed() > 20.0f;
    DribbleTouchAccumulator -= DeltaSeconds;

    if (DribbleTouchAccumulator <= 0.0f)
    {
        LastDribbleFoot = LastDribbleFoot == EFootballContactFoot::Left
            ? EFootballContactFoot::Right
            : EFootballContactFoot::Left;

        EFootballContactFoot TouchFoot = LastDribbleFoot;
        const FVector TouchLocation = GetBallContactLocation(TouchFoot);
        if (FVector::DistSquared2D(
            TouchLocation, ControlledBall->GetActorLocation()
        ) > FMath::Square(120.0f))
        {
            TouchFoot = TouchFoot == EFootballContactFoot::Left
                ? EFootballContactFoot::Right
                : EFootballContactFoot::Left;
        }

        ActiveContactFoot = TouchFoot;
        for (TActorIterator<AFootballMatchDirector> It(GetWorld()); It; ++It)
        {
            if (IsValid(*It) && It->NotifyBallTouched(this))
            {
                ControlledBall = nullptr;
                bHasBallControl = false;
                bIsDribbling = false;
                return;
            }
            break;
        }
        if (ControlledBall->ApplyFootTouch(
            GetBallContactLocation(TouchFoot), GetActorForwardVector(), GetVelocity()
        ))
        {
            ControlledBall->RegisterLastTouch(TeamId, this);
        }

        DribbleTouchAccumulator = bIsSprinting ? 0.20f : 0.28f;
    }
}

UAnimMontage* AFootballPlayerCharacter::GetBallActionMontage(
    const EFootballBallAction Action
) const
{
    switch (Action)
    {
    case EFootballBallAction::ShortPass:
        return PassMontage;
    case EFootballBallAction::ThroughBall:
        return ThroughBallMontage;
    case EFootballBallAction::Shot:
        return ShotMontage;
    default:
        return nullptr;
    }
}

float AFootballPlayerCharacter::PlayBallActionMontage(const EFootballBallAction Action)
{
    UAnimMontage* Montage = GetBallActionMontage(Action);
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

    if (!IsValid(Montage) || !IsValid(AnimInstance))
    {
        return 0.0f;
    }

    return AnimInstance->Montage_Play(Montage, 1.0f);
}

void AFootballPlayerCharacter::QueueBallAction(const EFootballBallAction Action)
{
    if (bIsContestStunned || PendingBallAction != EFootballBallAction::None)
    {
        return;
    }

    if (!IsValid(ControlledBall))
    {
        RefreshNearbyBall();
    }
    if (!IsValid(ControlledBall))
    {
        return;
    }

    if (FVector::DistSquared2D(
        GetActorLocation(), ControlledBall->GetActorLocation()
    ) > FMath::Square(300.0f))
    {
        ControlledBall = nullptr;
        return;
    }

    PendingBallAction = Action;
    PendingContactFoot = ChooseNearestFoot();
    ActiveBallAction = Action;
    ActiveContactFoot = PendingContactFoot;

    const float MontageLength = PlayBallActionMontage(Action);
    const float StateDuration = MontageLength > 0.0f
        ? MontageLength + 0.08f
        : 0.55f;
    const float ContactWatchdog = MontageLength > 0.0f
        ? FMath::Max(0.45f, MontageLength * 0.90f)
        : FMath::Max(0.12f, ActionContactFallbackDelay);

    GetWorldTimerManager().SetTimer(
        BallActionStateTimer,
        this,
        &AFootballPlayerCharacter::EndBallActionState,
        StateDuration,
        false
    );

    // The montage notify should fire first. This watchdog lets a configured
    // montage remain playable even if the asset author forgot to add the notify.
    GetWorldTimerManager().SetTimer(
        PendingContactFallbackTimer,
        this,
        &AFootballPlayerCharacter::OnBallContactFallback,
        ContactWatchdog,
        false
    );
}

void AFootballPlayerCharacter::HandleBallContactNotify(
    const EFootballContactFoot ContactFoot
)
{
    if (PendingBallAction == EFootballBallAction::None)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(PendingContactFallbackTimer);

    const EFootballContactFoot ResolvedFoot =
        ContactFoot == EFootballContactFoot::Auto
        ? PendingContactFoot
        : ContactFoot;

    ActiveContactFoot = ResolvedFoot;
    ExecutePendingBallAction(ResolvedFoot);
}

void AFootballPlayerCharacter::ExecutePendingBallAction(
    const EFootballContactFoot ContactFoot
)
{
    const EFootballBallAction Action = PendingBallAction;
    PendingBallAction = EFootballBallAction::None;

    if (!IsValid(ControlledBall))
    {
        RefreshNearbyBall();
    }
    if (!IsValid(ControlledBall))
    {
        return;
    }

    float SpeedCmPerSec = 0.0f;
    float LiftCmPerSec = 0.0f;
    float SpinDegPerSec = 0.0f;

    switch (Action)
    {
    case EFootballBallAction::ShortPass:
        SpeedCmPerSec = 1500.0f;
        LiftCmPerSec = 22.0f;
        SpinDegPerSec = 80.0f;
        break;
    case EFootballBallAction::ThroughBall:
        SpeedCmPerSec = 1900.0f;
        LiftCmPerSec = 65.0f;
        SpinDegPerSec = 130.0f;
        break;
    case EFootballBallAction::Shot:
        SpeedCmPerSec = 2550.0f;
        LiftCmPerSec = 420.0f;
        SpinDegPerSec = 260.0f;
        break;
    default:
        return;
    }

    AFootballMatchDirector* MatchDirector = nullptr;
    for (TActorIterator<AFootballMatchDirector> It(GetWorld()); It; ++It)
    {
        MatchDirector = *It;
        break;
    }
    if (IsValid(MatchDirector) && MatchDirector->IsRestartPendingForTeam(TeamId))
    {
        switch (MatchDirector->GetRestartType())
        {
        case EFootballRestartType::ThrowIn:
            SpeedCmPerSec = 1080.0f; LiftCmPerSec = 500.0f; break;
        case EFootballRestartType::CornerKick:
            SpeedCmPerSec = 1800.0f; LiftCmPerSec = 480.0f; break;
        case EFootballRestartType::GoalKick:
            SpeedCmPerSec = 2450.0f; LiftCmPerSec = 650.0f; break;
        case EFootballRestartType::DirectFreeKick:
        case EFootballRestartType::IndirectFreeKick:
            LiftCmPerSec = FMath::Max(LiftCmPerSec, 125.0f); break;
        case EFootballRestartType::PenaltyKick:
            SpeedCmPerSec = 2750.0f; LiftCmPerSec = 95.0f; SpinDegPerSec = 210.0f; break;
        default:
            break;
        }
    }

    // A player who is off balance can still attempt the action, but loses some
    // strike power. A direct contest stun blocks starting a fresh action above.
    const float BalancePowerScale = FMath::Lerp(0.72f, 1.0f, Balance);
    SpeedCmPerSec *= BalancePowerScale;
    LiftCmPerSec *= BalancePowerScale;

    const FVector FootLocation = GetBallContactLocation(ContactFoot);
    const bool bKicked = ControlledBall->ApplyKickFromFoot(
        FootLocation,
        GetActorForwardVector(),
        SpeedCmPerSec,
        LiftCmPerSec,
        SpinDegPerSec
    );

    if (bKicked)
    {
        ControlledBall->RegisterLastTouch(TeamId, this);
        if (IsValid(MatchDirector))
        {
            EFootballTacticalAction TacticalAction = EFootballTacticalAction::ShortPass;
            if (Action == EFootballBallAction::ThroughBall) TacticalAction = EFootballTacticalAction::ThroughBall;
            else if (Action == EFootballBallAction::Shot) TacticalAction = EFootballTacticalAction::Shot;
            if (MatchDirector->IsRestartPendingForTeam(TeamId))
            {
                switch (MatchDirector->GetRestartType())
                {
                case EFootballRestartType::ThrowIn: TacticalAction = EFootballTacticalAction::ThrowIn; break;
                case EFootballRestartType::CornerKick: TacticalAction = EFootballTacticalAction::CornerKick; break;
                case EFootballRestartType::GoalKick: TacticalAction = EFootballTacticalAction::GoalKick; break;
                case EFootballRestartType::DirectFreeKick:
                case EFootballRestartType::IndirectFreeKick: TacticalAction = EFootballTacticalAction::FreeKick; break;
                case EFootballRestartType::PenaltyKick: TacticalAction = EFootballTacticalAction::PenaltyKick; break;
                default: break;
                }
            }
            MatchDirector->NotifyBallPlayed(this, TacticalAction);
        }
        ControlledBall = nullptr;
        bHasBallControl = false;
        bIsDribbling = false;
        BallControlLockout = 0.70f;
        BallSearchAccumulator = 0.70f;
    }
}

void AFootballPlayerCharacter::OnBallContactFallback()
{
    HandleBallContactNotify(EFootballContactFoot::Auto);
}

void AFootballPlayerCharacter::EndBallActionState()
{
    ActiveBallAction = EFootballBallAction::None;
}

void AFootballPlayerCharacter::StartPass()
{
    QueueBallAction(EFootballBallAction::ShortPass);
}

void AFootballPlayerCharacter::StartThroughBall()
{
    QueueBallAction(EFootballBallAction::ThroughBall);
}

void AFootballPlayerCharacter::StartShot()
{
    QueueBallAction(EFootballBallAction::Shot);
}
