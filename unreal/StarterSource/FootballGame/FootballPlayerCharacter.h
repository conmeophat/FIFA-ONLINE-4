#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FootballPlayerCharacter.generated.h"

class AFootballBallActor;
class UCameraComponent;
class UMaterialInterface;
class USkeletalMesh;
class USpringArmComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UAnimInstance;

/**
 * First controllable footballer prototype.
 *
 * Input is camera-relative (controller yaw). A temporary primitive mannequin is
 * shown until a licensed/imported skeletal mesh is assigned in the editor.
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    FVector SkeletalMeshRelativeLocation = FVector(0.0f, 0.0f, -90.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    FRotator SkeletalMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    TObjectPtr<UMaterialInterface> ProxyKitMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    TObjectPtr<UMaterialInterface> ProxyShortsMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Appearance")
    TObjectPtr<UMaterialInterface> ProxyBootsMaterial = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement")
    bool bIsSprinting = false;

    UFUNCTION(BlueprintPure, Category = "Football|Animation")
    float GetHorizontalSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Football|Animation")
    float GetMovementDirection() const;

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

    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartSprint();
    void StopSprint();
    void StartJump();
    void StopJump();
    void StartPass();
    void StartThroughBall();
    void StartShot();
    void RefreshNearbyBall();
    void UpdateBallControl(float DeltaSeconds);
    void TryKickBall(float SpeedCmPerSec, float LiftCmPerSec, float SpinDegPerSec);

    UPROPERTY(Transient)
    TObjectPtr<AFootballBallActor> ControlledBall = nullptr;

    float BallSearchAccumulator = 0.0f;
    float BallControlLockout = 0.0f;
    void RefreshAppearance();
    void UpdateSprintSpeed();
};
