#pragma once

#include "CoreMinimal.h"
#include "FootballGameplayTypes.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "FootballPlayerCharacter.generated.h"

class AFootballBallActor;
class UAnimInstance;
class UAnimMontage;
class UCameraComponent;
class UMaterialInterface;
class USkeletalMesh;
class USpringArmComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Controllable footballer with socket-driven ball contacts.
 *
 * The contact socket names are editable because imported skeletons use different
 * naming conventions. If no socket exists, a conservative capsule-relative
 * fallback is used so the development mannequin remains playable.
 */
UCLASS(Blueprintable)
class FOOTBALLGAME_API AFootballPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFootballPlayerCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (ClampMin = "0"))
    int32 TeamId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (ClampMin = "-1", ClampMax = "1"))
    int32 AttackDirectionSign = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team")
    EFootballPlayerRole TeamRole = EFootballPlayerRole::Midfielder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team")
    EFootballFormation TeamFormation = EFootballFormation::Formation433;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (ClampMin = "0", ClampMax = "10"))
    int32 SquadSlotIndex = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "100.0"))
    float WalkSpeedCmPerSec = 480.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "1.0"))
    float SprintSpeedMultiplier = 1.55f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "100.0"))
    float AccelerationCmPerSec2 = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "100.0"))
    float BrakingDecelerationCmPerSec2 = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Camera")
    float InitialCameraPitch = -12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Camera", meta = (ClampMin = "100.0"))
    float CameraArmLengthCm = 620.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    TObjectPtr<USkeletalMesh> PlayerSkeletalMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    TSubclassOf<UAnimInstance> PlayerAnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    TObjectPtr<UAnimMontage> PassMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    TObjectPtr<UAnimMontage> ThroughBallMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    TObjectPtr<UAnimMontage> ShotMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    FName LeftFootContactSocketName = TEXT("ball_contact_l");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    FName RightFootContactSocketName = TEXT("ball_contact_r");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Animation", meta = (ClampMin = "0.0"))
    float ActionContactFallbackDelay = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "30.0"))
    float WalkingTurnRateDegreesPerSec = 560.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "30.0"))
    float SprintTurnRateDegreesPerSec = 380.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "100.0"))
    float BallFacingRangeCm = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (ClampMin = "0.0", ClampMax = "0.8"))
    float DirectionChangeSpeedLoss = 0.38f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "100.0"))
    float LateralAccelerationBalanceThreshold = 1450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0"))
    float BalanceLossPerSecondAtThreshold = 0.28f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0"))
    float BalanceRecoveryPerSecond = 0.34f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BraceContestResistance = 0.58f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BraceMovementMultiplier = 0.62f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OffBalanceSpeedMultiplier = 0.58f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement")
    bool bIsSprinting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement")
    bool bIsChangingDirection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Balance")
    bool bIsBracing = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Balance")
    bool bIsOffBalance = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Balance")
    bool bIsContestStunned = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Contest")
    bool bIsInShoulderDuel = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Contest", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ContestReactionStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Contest")
    bool bEnableAutomaticPlayerContests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Contest", meta = (ClampMin = "70.0", ClampMax = "160.0"))
    float ContestContactRadiusCm = 98.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Balance = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Balance")
    EFootballBalanceState BalanceState = EFootballBalanceState::Stable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Control")
    bool bHasBallControl = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Control")
    bool bIsDribbling = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    EFootballBallAction ActiveBallAction = EFootballBallAction::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Animation")
    EFootballContactFoot ActiveContactFoot = EFootballContactFoot::Right;

    UFUNCTION(BlueprintPure, Category = "Football|Animation")
    float GetHorizontalSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Football|Animation")
    float GetMovementDirection() const;

    /** Called by UAnimNotify_FootballBallContact on the montage's strike frame. */
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void HandleBallContactNotify(EFootballContactFoot ContactFoot);

    UFUNCTION(BlueprintPure, Category = "Football|Ball")
    FVector GetBallContactLocation(EFootballContactFoot ContactFoot) const;

    /**
     * Called by an opponent / overlap / tackle system when a contest contact occurs.
     * ImpulseCmPerSec is applied as a velocity change; brace reduces both push and balance loss.
     */
    /** AI pass / through-ball / cross / shot using the physical ball and a selected target. */
    UFUNCTION(BlueprintCallable, Category = "Football|AI")
    bool TryExecuteAITacticalKick(
        EFootballTacticalAction Action,
        const FVector& TargetLocation
    );

    UFUNCTION(BlueprintCallable, Category = "Football|Contest")
    void ApplyContestImpact(
        const FVector& ImpactDirection,
        float ImpulseCmPerSec,
        float BalanceDamage
    );

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> TorsoProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> HeadProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> ShortsProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> LeftArmProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> RightArmProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> LeftLegProxy;

    UPROPERTY(VisibleAnywhere, Category = "Football|Appearance")
    TObjectPtr<UStaticMeshComponent> RightLegProxy;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> ProxyCubeMesh;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> ProxySphereMesh;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> ProxyCylinderMesh;

    UPROPERTY(Transient)
    TObjectPtr<AFootballBallActor> ControlledBall = nullptr;

    EFootballBallAction PendingBallAction = EFootballBallAction::None;
    EFootballContactFoot PendingContactFoot = EFootballContactFoot::Right;
    FTimerHandle PendingContactFallbackTimer;
    FTimerHandle BallActionStateTimer;

    float BallSearchAccumulator = 0.0f;
    float BallControlLockout = 0.0f;
    float DribbleTouchAccumulator = 0.0f;
    float ContestStunRemaining = 0.0f;
    float ContestReactionRemaining = 0.0f;
    float PlayerContestScanAccumulator = 0.0f;
    float PlayerContestCooldown = 0.0f;
    FVector PreviousVelocity = FVector::ZeroVector;

    EFootballContactFoot LastDribbleFoot = EFootballContactFoot::Right;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartSprint();
    void StopSprint();
    void StartJump();
    void StopJump();

    void StartPass();
    void StartThroughBall();
    void StartShot();

    void QueueBallAction(EFootballBallAction Action);
    float PlayBallActionMontage(EFootballBallAction Action);
    UAnimMontage* GetBallActionMontage(EFootballBallAction Action) const;
    EFootballContactFoot ChooseNearestFoot() const;
    void RefreshNearbyBall();
    void UpdateBallControl(float DeltaSeconds);
    void ExecutePendingBallAction(EFootballContactFoot ContactFoot);
    void OnBallContactFallback();
    void EndBallActionState();

    void RefreshAppearance();
    void UpdateSprintSpeed();
    void UpdateFootballFacing(float DeltaSeconds);
    void UpdateBalance(float DeltaSeconds);
    void UpdatePlayerContests(float DeltaSeconds);
    void UpdateDynamicMovementTuning();
    void StartBrace();
    void StopBrace();
};
