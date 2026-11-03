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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement")
    bool bIsSprinting = false;

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
};
