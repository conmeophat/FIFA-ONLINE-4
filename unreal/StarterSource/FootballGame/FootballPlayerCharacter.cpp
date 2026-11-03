#include "FootballPlayerCharacter.h"

#include "FootballBallActor.h"

#include "Animation/AnimInstance.h"
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
    Movement->bOrientRotationToMovement = true;
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

    // A temporary, engine-built primitive mannequin keeps the pawn visible
    // before the project imports a properly licensed rigged footballer model.
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
            BallSearchAccumulator = 0.25f;
        }
    }
    if (IsValid(ControlledBall))
    {
        UpdateBallControl(DeltaSeconds);
    }
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
    PlayerInputComponent->BindAction(TEXT("Pass"), IE_Pressed, this, &AFootballPlayerCharacter::StartPass);
    PlayerInputComponent->BindAction(TEXT("ThroughBall"), IE_Pressed, this, &AFootballPlayerCharacter::StartThroughBall);
    PlayerInputComponent->BindAction(TEXT("Shoot"), IE_Pressed, this, &AFootballPlayerCharacter::StartShot);
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

void AFootballPlayerCharacter::UpdateSprintSpeed()
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        const float SafeMultiplier = FMath::Max(1.0f, SprintSpeedMultiplier);
        Movement->MaxWalkSpeed = bIsSprinting
            ? WalkSpeedCmPerSec * SafeMultiplier
            : WalkSpeedCmPerSec;
        Movement->MaxAcceleration = AccelerationCmPerSec2;
        Movement->BrakingDecelerationWalking = BrakingDecelerationCmPerSec2;
    }
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

    if (TorsoProxy)
    {
        TorsoProxy->SetMaterial(0, ProxyKitMaterial);
    }
    if (LeftArmProxy)
    {
        LeftArmProxy->SetMaterial(0, ProxyKitMaterial);
    }
    if (RightArmProxy)
    {
        RightArmProxy->SetMaterial(0, ProxyKitMaterial);
    }
    if (HeadProxy)
    {
        HeadProxy->SetMaterial(0, ProxyKitMaterial);
    }
    if (ShortsProxy)
    {
        ShortsProxy->SetMaterial(0, ProxyShortsMaterial);
    }
    if (LeftLegProxy)
    {
        LeftLegProxy->SetMaterial(0, ProxyBootsMaterial);
    }
    if (RightLegProxy)
    {
        RightLegProxy->SetMaterial(0, ProxyBootsMaterial);
    }
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


void AFootballPlayerCharacter::RefreshNearbyBall()
{
    if (!GetWorld())
    {
        return;
    }
    constexpr float SearchRangeCm = 300.0f;
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
    if (!IsValid(ControlledBall) || GetCharacterMovement()->IsFalling())
    {
        ControlledBall = nullptr;
        return;
    }
    const FVector ToBall = ControlledBall->GetActorLocation() - GetActorLocation();
    if (ToBall.SizeSquared2D() > FMath::Square(280.0f) ||
        FVector::DotProduct(GetActorForwardVector(), ToBall.GetSafeNormal2D()) < -0.35f)
    {
        ControlledBall = nullptr;
        return;
    }

    FVector TargetLocation = GetActorLocation() + GetActorForwardVector() * 125.0f;
    TargetLocation += GetVelocity().GetSafeNormal2D() * 25.0f;
    TargetLocation.Z = ControlledBall->GetActorLocation().Z;
    ControlledBall->ApplyDribbleControl(TargetLocation, GetVelocity(), DeltaSeconds);
}

void AFootballPlayerCharacter::TryKickBall(
    const float SpeedCmPerSec,
    const float LiftCmPerSec,
    const float SpinDegPerSec
)
{
    if (!IsValid(ControlledBall))
    {
        if (BallControlLockout > 0.0f)
        {
            return;
        }
        RefreshNearbyBall();
    }
    if (!IsValid(ControlledBall))
    {
        return;
    }

    const FVector ToBall = ControlledBall->GetActorLocation() - GetActorLocation();
    if (ToBall.SizeSquared2D() > FMath::Square(300.0f))
    {
        ControlledBall = nullptr;
        return;
    }

    if (ControlledBall->ApplyKick(
        GetActorForwardVector(), SpeedCmPerSec, LiftCmPerSec, SpinDegPerSec
    ))
    {
        ControlledBall = nullptr;
        BallControlLockout = 0.7f;
        BallSearchAccumulator = 0.7f;
    }
}

void AFootballPlayerCharacter::StartPass()
{
    TryKickBall(1500.0f, 22.0f, 80.0f);
}

void AFootballPlayerCharacter::StartThroughBall()
{
    TryKickBall(1900.0f, 65.0f, 130.0f);
}

void AFootballPlayerCharacter::StartShot()
{
    TryKickBall(2550.0f, 420.0f, 260.0f);
}
