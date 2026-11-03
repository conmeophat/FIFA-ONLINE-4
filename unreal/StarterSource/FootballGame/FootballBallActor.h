#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "FootballBallActor.generated.h"

class UPhysicalMaterial;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnFootballGoalScored,
    int32, GoalSide,
    int32, PositiveXScore,
    int32, NegativeXScore
);

/** Physics-driven football. The pitch runs along X; +1/-1 identify the goal end. */
UCLASS(BlueprintType)
class FOOTBALLGAME_API AFootballBallActor : public AActor
{
    GENERATED_BODY()

public:
    AFootballBallActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball")
    TObjectPtr<USphereComponent> PhysicsSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball")
    TObjectPtr<UStaticMeshComponent> BallVisual;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Physics", meta = (ClampMin = "1.0"))
    float BallRadiusCm = 11.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Physics", meta = (ClampMin = "0.1"))
    float BallMassKg = 0.43f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Physics", meta = (ClampMin = "0.0"))
    float LinearDamping = 0.16f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Physics", meta = (ClampMin = "0.0"))
    float AngularDamping = 0.32f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "1000.0"))
    float PitchHalfLengthCm = 5250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "100.0"))
    float GoalWidthCm = 732.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "100.0"))
    float GoalHeightCm = 244.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Ball Control", meta = (ClampMin = "1.0"))
    float FootTouchRangeCm = 125.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Football|Ball Control", meta = (ClampMin = "1.0"))
    float KickContactRangeCm = 150.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Score")
    int32 PositiveXScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Score")
    int32 NegativeXScore = 0;

    UPROPERTY(BlueprintAssignable, Category = "Football|Score")
    FOnFootballGoalScored OnGoalScored;

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    bool ApplyKick(const FVector& Direction, float SpeedCmPerSec, float LiftCmPerSec, float SpinDegPerSec);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    bool ApplyKickFromFoot(
        const FVector& FootLocation,
        const FVector& Direction,
        float SpeedCmPerSec,
        float LiftCmPerSec,
        float SpinDegPerSec
    );

    /** Apply one physical close-control touch from the animated foot socket. */
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    bool ApplyFootTouch(
        const FVector& FootLocation,
        const FVector& PlayerForward,
        const FVector& PlayerVelocity
    );

    /** Kept for compatibility with the v0.3 API. */
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ApplyDribbleControl(const FVector& TargetLocation, const FVector& PlayerVelocity, float DeltaSeconds);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UPhysicalMaterial> BallPhysicalMaterial;

    FVector PreviousBallLocation = FVector::ZeroVector;
    FTimerHandle GoalResetTimer;
    bool bWaitingForGoalReset = false;

    void CheckForGoal();
    bool IsInsideGoalMouth(const FVector& BallLocation) const;
    void RegisterGoal(int32 GoalSide);
    void ResetAfterGoal();
};
