#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FootballAITypes.h"
#include "FootballTeamAIController.generated.h"

class AFootballPlayerCharacter;

/** Lightweight, team-aware football AI using direct steering (no NavMesh required). */
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
    float PressingRadiusCm = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "50.0"))
    float ContestDistanceCm = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI")
    bool bEnableAutomaticContestImpacts = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float FieldHalfLengthCm = 5250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|AI", meta = (ClampMin = "100.0"))
    float FieldHalfWidthCm = 3400.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|AI")
    EFootballAITaskState CurrentTaskState = EFootballAITaskState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|AI")
    TObjectPtr<AFootballPlayerCharacter> CurrentBallCarrier = nullptr;

    UFUNCTION(BlueprintPure, Category = "Football|AI")
    EFootballAITaskState GetCurrentTaskState() const { return CurrentTaskState; }

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

private:
    float DecisionAccumulator = 0.0f;
    float ContestCooldown = 0.0f;
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
    FVector FindSpaceTarget(
        const FVector& BallLocation,
        const TArray<AFootballPlayerCharacter*>& Teammates,
        const TArray<AFootballPlayerCharacter*>& Opponents,
        int32 AttackDirectionSign,
        EFootballPlayerRole Role
    ) const;
    void MoveTowards(const FVector& TargetLocation, const TArray<AFootballPlayerCharacter*>& Teammates);
    void ApplyCloseContest(AFootballPlayerCharacter* Opponent);
};
