#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "FootballAITypes.h"
#include "FootballMatchDirector.generated.h"

class AFootballBallActor;
class AFootballPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootballRestartAwarded, EFootballRestartType, RestartType, int32, RestartTeamId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootballMatchScoreChanged, int32, TeamZeroScore, int32, TeamOneScore);

/** Match clock, team score and basic referee authority for a two-team offline match. */
UCLASS(BlueprintType)
class FOOTBALLGAME_API AFootballMatchDirector : public AActor
{
    GENERATED_BODY()

public:
    AFootballMatchDirector();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (ClampMin = "60.0"))
    float HalfDurationSeconds = 2700.0f;

    /** 1 is real time; 30 means one match minute every two real seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (ClampMin = "0.1", ClampMax = "120.0"))
    float MatchClockRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (ClampMin = "0.0", ClampMax = "120.0"))
    float HalfTimeBreakSeconds = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Pitch", meta = (ClampMin = "1000.0"))
    float PitchHalfLengthCm = 5250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Pitch", meta = (ClampMin = "1000.0"))
    float PitchHalfWidthCm = 3400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Pitch", meta = (ClampMin = "100.0"))
    float GoalWidthCm = 732.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Pitch", meta = (ClampMin = "100.0"))
    float GoalHeightCm = 244.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match")
    EFootballMatchPeriod CurrentPeriod = EFootballMatchPeriod::PreMatch;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match")
    float MatchClockSeconds = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match")
    bool bClockRunning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Score")
    int32 TeamZeroScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Score")
    int32 TeamOneScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee")
    bool bRestartPending = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee")
    EFootballRestartType PendingRestartType = EFootballRestartType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee")
    int32 RestartTeamId = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee")
    FVector RestartLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintAssignable, Category = "Football|Referee")
    FOnFootballRestartAwarded OnRestartAwarded;

    UPROPERTY(BlueprintAssignable, Category = "Football|Score")
    FOnFootballMatchScoreChanged OnScoreChanged;

    UFUNCTION(BlueprintPure, Category = "Football|Match")
    bool IsMatchActive() const;

    UFUNCTION(BlueprintPure, Category = "Football|Match")
    FString GetMatchClockText() const;

    UFUNCTION(BlueprintPure, Category = "Football|Referee")
    bool IsRestartPendingForTeam(int32 TeamId) const;

    UFUNCTION(BlueprintPure, Category = "Football|Referee")
    EFootballRestartType GetRestartType() const { return PendingRestartType; }

    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    void NotifyBallPlayed(AFootballPlayerCharacter* Player, EFootballTacticalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    bool NotifyBallTouched(AFootballPlayerCharacter* Player);

    /** Blueprint foul hook; tackle/collision logic supplies the foul location and penalty-area result. */
    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    void SignalFoul(AFootballPlayerCharacter* Offender, const FVector& FoulLocation, bool bInsidePenaltyArea);

    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    bool NotifyRestartTaken(AFootballPlayerCharacter* RestartingPlayer);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<AFootballBallActor> MatchBall = nullptr;

    FTimerHandle HalfTimeTimer;
    float CurrentPeriodElapsedSeconds = 0.0f;
    TSet<TWeakObjectPtr<AFootballPlayerCharacter>> OffsideCandidates;

    UFUNCTION()
    void HandleGoalScored(int32 GoalSide, int32 PositiveXScore, int32 NegativeXScore);

    void FindMatchBall();
    void CheckOutOfPlay();
    void SetRestart(EFootballRestartType RestartType, int32 AwardedTeamId, const FVector& Location);
    void EnterHalfTime();
    void StartSecondHalf();
    void FlipTeamDirections();
    void BuildOffsideSnapshot(AFootballPlayerCharacter* Passer);
    int32 FindTeamAttackingToward(int32 GoalSide) const;
    bool IsGoalMouthTrajectory(const FVector& BallLocation) const;
};
