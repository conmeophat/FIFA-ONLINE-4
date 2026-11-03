#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FootballAITypes.h"
#include "FootballTeamAIController.generated.h"

class AFootballPlayerCharacter;

/**
 * Team-aware tactical AI. Each controller evaluates the shared team shape,
 * while movement steering is applied continuously between tactical decisions.
 */
UCLASS(Blueprintable)
class FOOTBALLGAME_API AFootballTeamAIController : public AAIController
{
    GENERATED_BODY()

public:
    AFootballTeamAIController();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float DecisionIntervalSeconds = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "50.0"))
    float StopDistanceCm = 115.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float SlowDownDistanceCm = 750.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float BallPossessionRadiusCm = 235.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float PressingRadiusCm = 1900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "50.0"))
    float ContestDistanceCm = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI")
    bool bEnableAutomaticContestImpacts = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactics", meta = (ClampMin = "500.0"))
    float ShotRangeCm = 2050.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactics", meta = (ClampMin = "300.0"))
    float MaximumPassRangeCm = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactics", meta = (ClampMin = "50.0"))
    float MinimumPassLaneClearanceCm = 145.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactics", meta = (ClampMin = "100.0"))
    float CrossZoneDepthCm = 1650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float FieldHalfLengthCm = 5250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float FieldHalfWidthCm = 3400.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|AI")
    EFootballAITaskState CurrentTaskState = EFootballAITaskState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactics")
    EFootballMatchPhase CurrentMatchPhase = EFootballMatchPhase::TransitionToAttack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactics")
    EFootballTacticalAction CurrentTacticalAction = EFootballTacticalAction::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|AI")
    TObjectPtr<AFootballPlayerCharacter> CurrentBallCarrier = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactics")
    TObjectPtr<AFootballPlayerCharacter> CurrentTacticalTarget = nullptr;

    UFUNCTION(BlueprintPure, Category = "Football|AI")
    EFootballAITaskState GetCurrentTaskState() const { return CurrentTaskState; }

    UFUNCTION(BlueprintPure, Category = "Football|Tactics")
    EFootballMatchPhase GetCurrentMatchPhase() const { return CurrentMatchPhase; }

    UFUNCTION(BlueprintPure, Category = "Football|Tactics")
    EFootballTacticalAction GetCurrentTacticalAction() const { return CurrentTacticalAction; }

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

private:
    float DecisionAccumulator = 0.0f;
    float ContestCooldown = 0.0f;
    float TacticalActionCooldown = 0.0f;
    FVector HomeLocation = FVector::ZeroVector;
    FVector CachedMoveTarget = FVector::ZeroVector;
    bool bHasCachedMoveTarget = false;
    TArray<TWeakObjectPtr<AFootballPlayerCharacter>> CachedTeammates;

    void UpdateTeamDecision();
    void ApplyCachedSteering();
    void GatherFootballers(TArray<AFootballPlayerCharacter*>& OutPlayers) const;
    AFootballPlayerCharacter* FindClosestPlayer(
        const TArray<AFootballPlayerCharacter*>& Players,
        const FVector& Location,
        float& OutDistance
    ) const;
    AFootballPlayerCharacter* FindMarkTarget(
        const TArray<AFootballPlayerCharacter*>& Opponents,
        const TArray<AFootballPlayerCharacter*>& Teammates,
        const FVector& BallLocation
    ) const;
    FVector FindFormationTarget(
        const FVector& BallLocation,
        const AFootballPlayerCharacter* SelfPlayer,
        bool bDefending
    ) const;
    EFootballPlayerRole ResolveRole(const AFootballPlayerCharacter* Player) const;
    float GetPassLaneClearance(
        const FVector& Start,
        const FVector& End,
        const TArray<AFootballPlayerCharacter*>& Opponents
    ) const;
    bool TryTacticalBallAction(
        AFootballPlayerCharacter* BallCarrier,
        const FVector& BallLocation,
        const TArray<AFootballPlayerCharacter*>& Teammates,
        const TArray<AFootballPlayerCharacter*>& Opponents,
        int32 AttackDirectionSign
    );
    void MoveTowards(const FVector& TargetLocation, const TArray<AFootballPlayerCharacter*>& Teammates);
    void ApplyCloseContest(AFootballPlayerCharacter* Opponent);
};
