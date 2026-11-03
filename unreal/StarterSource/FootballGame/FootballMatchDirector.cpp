#include "FootballMatchDirector.h"

#include "FootballBallActor.h"
#include "FootballPlayerCharacter.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AFootballMatchDirector::AFootballMatchDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void AFootballMatchDirector::BeginPlay()
{
    Super::BeginPlay();
    FindMatchBall();
    CurrentPeriod = EFootballMatchPeriod::FirstHalf;
    MatchClockSeconds = 0.0f;
    CurrentPeriodElapsedSeconds = 0.0f;
    bClockRunning = false;

    if (IsValid(MatchBall))
    {
        MatchBall->OnGoalScored.AddDynamic(this, &AFootballMatchDirector::HandleGoalScored);
        SetRestart(EFootballRestartType::KickOff, 0,
            FVector(0.0f, 0.0f, MatchBall->BallRadiusCm + 2.0f));
    }
}

void AFootballMatchDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsValid(MatchBall))
    {
        FindMatchBall();
    }

    if (bClockRunning && IsMatchActive())
    {
        const float MatchDelta = DeltaSeconds * FMath::Max(0.1f, MatchClockRate);
        CurrentPeriodElapsedSeconds += MatchDelta;
        MatchClockSeconds += MatchDelta;
        if (CurrentPeriod == EFootballMatchPeriod::FirstHalf &&
            CurrentPeriodElapsedSeconds >= HalfDurationSeconds)
        {
            EnterHalfTime();
        }
        else if (CurrentPeriod == EFootballMatchPeriod::SecondHalf &&
            CurrentPeriodElapsedSeconds >= HalfDurationSeconds)
        {
            CurrentPeriod = EFootballMatchPeriod::FullTime;
            bClockRunning = false;
            bRestartPending = false;
            PendingRestartType = EFootballRestartType::None;
            RestartTeamId = INDEX_NONE;
        }
    }

    if (!bRestartPending && IsMatchActive())
    {
        CheckOutOfPlay();
    }
}

void AFootballMatchDirector::FindMatchBall()
{
    if (!GetWorld()) return;
    for (TActorIterator<AFootballBallActor> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            MatchBall = *It;
            return;
        }
    }
}

bool AFootballMatchDirector::IsMatchActive() const
{
    return CurrentPeriod == EFootballMatchPeriod::FirstHalf ||
        CurrentPeriod == EFootballMatchPeriod::SecondHalf;
}

FString AFootballMatchDirector::GetMatchClockText() const
{
    const int32 Seconds = FMath::Max(0, FMath::FloorToInt(MatchClockSeconds));
    return FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60);
}

bool AFootballMatchDirector::IsRestartPendingForTeam(const int32 TeamId) const
{
    return bRestartPending && RestartTeamId == TeamId;
}

void AFootballMatchDirector::SetRestart(
    const EFootballRestartType RestartType,
    const int32 AwardedTeamId,
    const FVector& Location
)
{
    if (CurrentPeriod == EFootballMatchPeriod::FullTime) return;
    bRestartPending = true;
    PendingRestartType = RestartType;
    RestartTeamId = FMath::Clamp(AwardedTeamId, 0, 1);
    RestartLocation = Location;
    OffsideCandidates.Reset();
    if (IsValid(MatchBall))
    {
        MatchBall->StopAtRestart(RestartLocation);
    }
    OnRestartAwarded.Broadcast(PendingRestartType, RestartTeamId);
}

void AFootballMatchDirector::NotifyBallPlayed(
    AFootballPlayerCharacter* Player,
    const EFootballTacticalAction Action
)
{
    if (!IsValid(Player) || CurrentPeriod == EFootballMatchPeriod::FullTime) return;
    if (bRestartPending)
    {
        NotifyRestartTaken(Player);
    }

    if (Action == EFootballTacticalAction::ThrowIn ||
        Action == EFootballTacticalAction::CornerKick ||
        Action == EFootballTacticalAction::GoalKick)
    {
        OffsideCandidates.Reset();
        return;
    }

    if (Action == EFootballTacticalAction::ShortPass ||
        Action == EFootballTacticalAction::ThroughBall ||
        Action == EFootballTacticalAction::Cross ||
        Action == EFootballTacticalAction::FreeKick ||
        Action == EFootballTacticalAction::PenaltyKick ||
        Action == EFootballTacticalAction::Shot)
    {
        BuildOffsideSnapshot(Player);
    }
    else
    {
        OffsideCandidates.Reset();
    }
}

void AFootballMatchDirector::BuildOffsideSnapshot(AFootballPlayerCharacter* Passer)
{
    OffsideCandidates.Reset();
    if (!IsValid(Passer) || !GetWorld()) return;
    const int32 Sign = Passer->AttackDirectionSign >= 0 ? 1 : -1;
    const float BallProgress = IsValid(MatchBall)
        ? MatchBall->GetActorLocation().X * static_cast<float>(Sign)
        : Passer->GetActorLocation().X * static_cast<float>(Sign);

    TArray<float> DefenderProgress;
    for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
    {
        AFootballPlayerCharacter* Defender = *It;
        if (IsValid(Defender) && Defender->TeamId != Passer->TeamId)
        {
            DefenderProgress.Add(Defender->GetActorLocation().X * static_cast<float>(Sign));
        }
    }
    DefenderProgress.Sort([](const float A, const float B) { return A > B; });

    float SecondLastDefender = 0.0f;
    if (DefenderProgress.Num() >= 2) SecondLastDefender = DefenderProgress[1];
    else if (DefenderProgress.Num() == 1) SecondLastDefender = DefenderProgress[0];
    const float OffsideLine = FMath::Max(BallProgress, SecondLastDefender);

    for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
    {
        AFootballPlayerCharacter* Attacker = *It;
        if (!IsValid(Attacker) || Attacker == Passer || Attacker->TeamId != Passer->TeamId) continue;
        const float Progress = Attacker->GetActorLocation().X * static_cast<float>(Sign);
        if (Progress > 0.0f && Progress > OffsideLine + 5.0f)
        {
            OffsideCandidates.Add(TWeakObjectPtr<AFootballPlayerCharacter>(Attacker));
        }
    }
}

bool AFootballMatchDirector::NotifyBallTouched(AFootballPlayerCharacter* Player)
{
    if (!IsValid(Player) || bRestartPending || OffsideCandidates.IsEmpty()) return false;
    if (OffsideCandidates.Contains(TWeakObjectPtr<AFootballPlayerCharacter>(Player)))
    {
        const int32 AwardedTeam = Player->TeamId == 0 ? 1 : 0;
        const float BallZ = IsValid(MatchBall) ? MatchBall->BallRadiusCm + 2.0f : 13.0f;
        SetRestart(EFootballRestartType::IndirectFreeKick, AwardedTeam,
            FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, BallZ));
        return true;
    }
    OffsideCandidates.Reset();
    return false;
}

void AFootballMatchDirector::SignalFoul(
    AFootballPlayerCharacter* Offender,
    const FVector& FoulLocation,
    const bool bInsidePenaltyArea
)
{
    if (!IsValid(Offender)) return;
    const int32 AwardedTeam = Offender->TeamId == 0 ? 1 : 0;
    int32 AwardSign = -Offender->AttackDirectionSign;
    if (GetWorld())
    {
        for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
        {
            if (IsValid(*It) && It->TeamId == AwardedTeam)
            {
                AwardSign = It->AttackDirectionSign >= 0 ? 1 : -1;
                break;
            }
        }
    }

    const float BallZ = IsValid(MatchBall) ? MatchBall->BallRadiusCm + 2.0f : 13.0f;
    FVector Spot = FoulLocation;
    if (bInsidePenaltyArea)
    {
        Spot.X = static_cast<float>(AwardSign) * (PitchHalfLengthCm - 1050.0f);
        Spot.Y = FMath::Clamp(Spot.Y, -650.0f, 650.0f);
        SetRestart(EFootballRestartType::PenaltyKick, AwardedTeam, FVector(Spot.X, Spot.Y, BallZ));
    }
    else
    {
        Spot.X = FMath::Clamp(Spot.X, -PitchHalfLengthCm + 250.0f, PitchHalfLengthCm - 250.0f);
        Spot.Y = FMath::Clamp(Spot.Y, -PitchHalfWidthCm + 250.0f, PitchHalfWidthCm - 250.0f);
        SetRestart(EFootballRestartType::DirectFreeKick, AwardedTeam, FVector(Spot.X, Spot.Y, BallZ));
    }
}

bool AFootballMatchDirector::NotifyRestartTaken(AFootballPlayerCharacter* Player)
{
    if (!bRestartPending || !IsValid(Player) || Player->TeamId != RestartTeamId ||
        !IsValid(MatchBall) ||
        FVector::Dist2D(Player->GetActorLocation(), MatchBall->GetActorLocation()) > 520.0f)
    {
        return false;
    }
    bRestartPending = false;
    PendingRestartType = EFootballRestartType::None;
    RestartTeamId = INDEX_NONE;
    if (IsMatchActive()) bClockRunning = true;
    return true;
}

void AFootballMatchDirector::CheckOutOfPlay()
{
    if (!IsValid(MatchBall)) return;
    const FVector Location = MatchBall->GetActorLocation();
    const float Radius = MatchBall->BallRadiusCm;
    if (IsGoalMouthTrajectory(Location)) return;

    const bool bTouchline = FMath::Abs(Location.Y) > PitchHalfWidthCm + Radius;
    const bool bPositiveEnd = Location.X > PitchHalfLengthCm + Radius;
    const bool bNegativeEnd = Location.X < -PitchHalfLengthCm - Radius;
    if (!bTouchline && !bPositiveEnd && !bNegativeEnd) return;

    const int32 LastTouch = MatchBall->LastTouchTeamId;
    if (bTouchline)
    {
        const int32 AwardTeam = LastTouch == 0 ? 1 : 0;
        const float Side = Location.Y >= 0.0f ? 1.0f : -1.0f;
        SetRestart(EFootballRestartType::ThrowIn, AwardTeam,
            FVector(FMath::Clamp(Location.X, -PitchHalfLengthCm + 100.0f, PitchHalfLengthCm - 100.0f),
                Side * (PitchHalfWidthCm - 35.0f), Radius + 2.0f));
        return;
    }

    const int32 GoalSide = bPositiveEnd ? 1 : -1;
    const int32 AttackTeam = FindTeamAttackingToward(GoalSide);
    const int32 DefendTeam = AttackTeam == 0 ? 1 : 0;
    const float Side = Location.Y >= 0.0f ? 1.0f : -1.0f;
    if (LastTouch == AttackTeam)
    {
        SetRestart(EFootballRestartType::GoalKick, DefendTeam,
            FVector(static_cast<float>(GoalSide) * (PitchHalfLengthCm - 900.0f), 0.0f, Radius + 2.0f));
    }
    else
    {
        SetRestart(EFootballRestartType::CornerKick, AttackTeam,
            FVector(static_cast<float>(GoalSide) * (PitchHalfLengthCm - 35.0f),
                Side * (PitchHalfWidthCm - 35.0f), Radius + 2.0f));
    }
}

bool AFootballMatchDirector::IsGoalMouthTrajectory(const FVector& Location) const
{
    return FMath::Abs(Location.Y) <= GoalWidthCm * 0.5f + 35.0f &&
        Location.Z >= 0.0f && Location.Z <= GoalHeightCm + 65.0f &&
        FMath::Abs(Location.X) <= PitchHalfLengthCm + 120.0f;
}

int32 AFootballMatchDirector::FindTeamAttackingToward(const int32 GoalSide) const
{
    if (GetWorld())
    {
        for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
        {
            if (IsValid(*It) && (It->AttackDirectionSign >= 0 ? 1 : -1) == GoalSide)
            {
                return FMath::Clamp(It->TeamId, 0, 1);
            }
        }
    }
    return GoalSide > 0 ? 0 : 1;
}

void AFootballMatchDirector::HandleGoalScored(
    const int32 GoalSide,
    const int32 PositiveXScore,
    const int32 NegativeXScore
)
{
    (void)PositiveXScore;
    (void)NegativeXScore;
    const int32 ScoringTeam = FindTeamAttackingToward(GoalSide);
    if (ScoringTeam == 0) ++TeamZeroScore;
    else ++TeamOneScore;
    OnScoreChanged.Broadcast(TeamZeroScore, TeamOneScore);

    bClockRunning = false;
    const int32 ConcedingTeam = ScoringTeam == 0 ? 1 : 0;
    SetRestart(EFootballRestartType::KickOff, ConcedingTeam,
        FVector(0.0f, 0.0f, IsValid(MatchBall) ? MatchBall->BallRadiusCm + 2.0f : 13.0f));
}

void AFootballMatchDirector::EnterHalfTime()
{
    bClockRunning = false;
    CurrentPeriod = EFootballMatchPeriod::HalfTime;
    MatchClockSeconds = HalfDurationSeconds;
    CurrentPeriodElapsedSeconds = 0.0f;
    FlipTeamDirections();
    SetRestart(EFootballRestartType::KickOff, 1,
        FVector(0.0f, 0.0f, IsValid(MatchBall) ? MatchBall->BallRadiusCm + 2.0f : 13.0f));
    GetWorldTimerManager().SetTimer(HalfTimeTimer, this,
        &AFootballMatchDirector::StartSecondHalf, FMath::Max(0.0f, HalfTimeBreakSeconds), false);
}

void AFootballMatchDirector::StartSecondHalf()
{
    if (CurrentPeriod != EFootballMatchPeriod::HalfTime) return;
    CurrentPeriod = EFootballMatchPeriod::SecondHalf;
    CurrentPeriodElapsedSeconds = 0.0f;
    MatchClockSeconds = HalfDurationSeconds;
    bClockRunning = false;
    SetRestart(EFootballRestartType::KickOff, 1,
        FVector(0.0f, 0.0f, IsValid(MatchBall) ? MatchBall->BallRadiusCm + 2.0f : 13.0f));
}

void AFootballMatchDirector::FlipTeamDirections()
{
    if (!GetWorld()) return;
    for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It)) It->AttackDirectionSign = It->AttackDirectionSign >= 0 ? -1 : 1;
    }
}
