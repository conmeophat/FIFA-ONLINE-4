#include "FootballTeamAIController.h"

#include "FootballBallActor.h"
#include "FootballMatchDirector.h"
#include "FootballPlayerCharacter.h"
#include "EngineUtils.h"

namespace
{
    FVector2D GetFormationOffset(
        const EFootballFormation Formation,
        const int32 SquadSlot
    )
    {
        const int32 Slot = FMath::Clamp(SquadSlot, 0, 10);

        // X is relative to the team's attacking direction; Y selects the lane.
        static const FVector2D Formation433[11] = {
            FVector2D(-4200.0f, 0.0f),
            FVector2D(-1550.0f, -1650.0f),
            FVector2D(-1750.0f, -540.0f),
            FVector2D(-1750.0f, 540.0f),
            FVector2D(-1550.0f, 1650.0f),
            FVector2D(-650.0f, 0.0f),
            FVector2D(-180.0f, -850.0f),
            FVector2D(-180.0f, 850.0f),
            FVector2D(900.0f, -1550.0f),
            FVector2D(1280.0f, 0.0f),
            FVector2D(900.0f, 1550.0f)
        };
        static const FVector2D Formation4231[11] = {
            FVector2D(-4200.0f, 0.0f),
            FVector2D(-1550.0f, -1650.0f),
            FVector2D(-1750.0f, -540.0f),
            FVector2D(-1750.0f, 540.0f),
            FVector2D(-1550.0f, 1650.0f),
            FVector2D(-850.0f, -520.0f),
            FVector2D(-850.0f, 520.0f),
            FVector2D(250.0f, 0.0f),
            FVector2D(500.0f, -1500.0f),
            FVector2D(1320.0f, 0.0f),
            FVector2D(500.0f, 1500.0f)
        };
        static const FVector2D Formation442[11] = {
            FVector2D(-4200.0f, 0.0f),
            FVector2D(-1550.0f, -1650.0f),
            FVector2D(-1750.0f, -540.0f),
            FVector2D(-1750.0f, 540.0f),
            FVector2D(-1550.0f, 1650.0f),
            FVector2D(180.0f, -1550.0f),
            FVector2D(-250.0f, -520.0f),
            FVector2D(-250.0f, 520.0f),
            FVector2D(180.0f, 1550.0f),
            FVector2D(1180.0f, -520.0f),
            FVector2D(1180.0f, 520.0f)
        };

        switch (Formation)
        {
        case EFootballFormation::Formation4231:
            return Formation4231[Slot];
        case EFootballFormation::Formation442:
            return Formation442[Slot];
        case EFootballFormation::Formation433:
        default:
            return Formation433[Slot];
        }
    }

    float DistancePointToSegment2D(
        const FVector& Point,
        const FVector& SegmentStart,
        const FVector& SegmentEnd
    )
    {
        FVector Segment = SegmentEnd - SegmentStart;
        Segment.Z = 0.0f;
        FVector ToPoint = Point - SegmentStart;
        ToPoint.Z = 0.0f;
        const float LengthSquared = Segment.SizeSquared();
        if (LengthSquared <= KINDA_SMALL_NUMBER)
        {
            return ToPoint.Size();
        }
        const float T = FMath::Clamp(
            FVector::DotProduct(ToPoint, Segment) / LengthSquared,
            0.0f,
            1.0f
        );
        return FVector::Dist2D(Point, SegmentStart + Segment * T);
    }
}

AFootballTeamAIController::AFootballTeamAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFootballTeamAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (IsValid(InPawn))
    {
        HomeLocation = InPawn->GetActorLocation();
    }
    DecisionAccumulator = 0.0f;
    TacticalActionCooldown = 0.0f;
    PhaseTransitionRemaining = 0.55f;
    LastStableMatchPhase = EFootballMatchPhase::Attacking;
    CurrentMatchPhase = EFootballMatchPhase::TransitionToAttack;
}

void AFootballTeamAIController::OnUnPossess()
{
    CurrentBallCarrier = nullptr;
    CurrentTacticalTarget = nullptr;
    CurrentTaskState = EFootballAITaskState::Idle;
    CurrentMatchPhase = EFootballMatchPhase::TransitionToAttack;
    CurrentTacticalAction = EFootballTacticalAction::None;
    bHasCachedMoveTarget = false;
    CachedTeammates.Reset();
    Super::OnUnPossess();
}

void AFootballTeamAIController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ContestCooldown = FMath::Max(0.0f, ContestCooldown - DeltaSeconds);
    TacticalActionCooldown = FMath::Max(0.0f, TacticalActionCooldown - DeltaSeconds);
    if (TacticalActionCooldown <= 0.0f)
    {
        CurrentTacticalAction = EFootballTacticalAction::None;
        CurrentTacticalTarget = nullptr;
    }
    DecisionAccumulator += DeltaSeconds;
    if (DecisionAccumulator >= FMath::Max(0.05f, DecisionIntervalSeconds))
    {
        DecisionAccumulator = 0.0f;
        UpdateTeamDecision();
    }

    // Tactical decisions are throttled, but movement input is applied each frame.
    ApplyCachedSteering();
}

void AFootballTeamAIController::GatherFootballers(
    TArray<AFootballPlayerCharacter*>& OutPlayers
) const
{
    OutPlayers.Reset();
    if (!GetWorld())
    {
        return;
    }
    for (TActorIterator<AFootballPlayerCharacter> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            OutPlayers.Add(*It);
        }
    }
}

AFootballPlayerCharacter* AFootballTeamAIController::FindClosestPlayer(
    const TArray<AFootballPlayerCharacter*>& Players,
    const FVector& Location,
    float& OutDistance
) const
{
    AFootballPlayerCharacter* Closest = nullptr;
    OutDistance = TNumericLimits<float>::Max();
    for (AFootballPlayerCharacter* Player : Players)
    {
        if (!IsValid(Player))
        {
            continue;
        }
        const float Distance = FVector::Dist2D(Player->GetActorLocation(), Location);
        if (Distance < OutDistance)
        {
            OutDistance = Distance;
            Closest = Player;
        }
    }
    return Closest;
}

EFootballPlayerRole AFootballTeamAIController::ResolveRole(
    const AFootballPlayerCharacter* Player
) const
{
    if (!IsValid(Player))
    {
        return EFootballPlayerRole::Midfielder;
    }
    if (Player->TeamRole != EFootballPlayerRole::Midfielder)
    {
        return Player->TeamRole;
    }

    const int32 Slot = FMath::Clamp(Player->SquadSlotIndex, 0, 10);
    if (Slot == 0)
    {
        return EFootballPlayerRole::Goalkeeper;
    }
    if (Slot >= 1 && Slot <= 4)
    {
        return EFootballPlayerRole::Defender;
    }
    if (Slot == 8 || Slot == 10)
    {
        return EFootballPlayerRole::Winger;
    }
    if (Slot == 9)
    {
        return EFootballPlayerRole::Forward;
    }
    return EFootballPlayerRole::Midfielder;
}

FVector AFootballTeamAIController::FindFormationTarget(
    const FVector& BallLocation,
    const AFootballPlayerCharacter* SelfPlayer,
    const bool bDefending
) const
{
    if (!IsValid(SelfPlayer))
    {
        return BallLocation;
    }

    FVector2D Offset = GetFormationOffset(
        SelfPlayer->TeamFormation,
        SelfPlayer->SquadSlotIndex
    );

    if (bDefending)
    {
        // Compress the team between the ball and its own goal while preserving lane width.
        Offset.X = Offset.X * 0.52f - 420.0f;
        Offset.Y *= 0.86f;
    }

    FVector Target = BallLocation + FVector(
        Offset.X * static_cast<float>(SelfPlayer->AttackDirectionSign >= 0 ? 1 : -1),
        Offset.Y,
        0.0f
    );

    if (ResolveRole(SelfPlayer) == EFootballPlayerRole::Goalkeeper)
    {
        Target = HomeLocation;
        Target.Y = FMath::Clamp(BallLocation.Y * 0.28f, -520.0f, 520.0f);
    }

    Target.X = FMath::Clamp(Target.X, -FieldHalfLengthCm + 250.0f, FieldHalfLengthCm - 250.0f);
    Target.Y = FMath::Clamp(Target.Y, -FieldHalfWidthCm + 250.0f, FieldHalfWidthCm - 250.0f);
    Target.Z = SelfPlayer->GetActorLocation().Z;
    return Target;
}

float AFootballTeamAIController::GetPassLaneClearance(
    const FVector& Start,
    const FVector& End,
    const TArray<AFootballPlayerCharacter*>& Opponents
) const
{
    float Clearance = 2600.0f;
    for (AFootballPlayerCharacter* Opponent : Opponents)
    {
        if (IsValid(Opponent))
        {
            Clearance = FMath::Min(
                Clearance,
                DistancePointToSegment2D(
                    Opponent->GetActorLocation(),
                    Start,
                    End
                )
            );
        }
    }
    return Clearance;
}

bool AFootballTeamAIController::TryTacticalBallAction(
    AFootballPlayerCharacter* BallCarrier,
    const FVector& BallLocation,
    const TArray<AFootballPlayerCharacter*>& Teammates,
    const TArray<AFootballPlayerCharacter*>& Opponents,
    const int32 AttackDirectionSign
)
{
    if (!IsValid(BallCarrier) ||
        !BallCarrier->bHasBallControl ||
        TacticalActionCooldown > 0.0f)
    {
        return false;
    }

    const FVector GoalLocation(
        static_cast<float>(AttackDirectionSign) * FieldHalfLengthCm,
        0.0f,
        BallLocation.Z
    );
    const float GoalDistance = FVector::Dist2D(BallLocation, GoalLocation);
    const float AttackProgress = BallLocation.X * static_cast<float>(AttackDirectionSign);

    if (GoalDistance <= ShotRangeCm &&
        FMath::Abs(BallLocation.Y) <= 2600.0f)
    {
        if (BallCarrier->TryExecuteAITacticalKick(
            EFootballTacticalAction::Shot,
            GoalLocation
        ))
        {
            CurrentTacticalAction = EFootballTacticalAction::Shot;
            CurrentTacticalTarget = nullptr;
            TacticalActionCooldown = 1.55f;
            return true;
        }
    }

    // A wide player in the attacking third looks for a central runner and crosses.
    const bool bInCrossingChannel =
        FMath::Abs(BallLocation.Y) >= FieldHalfWidthCm * 0.58f &&
        AttackProgress >= FieldHalfLengthCm - CrossZoneDepthCm;
    if (bInCrossingChannel)
    {
        AFootballPlayerCharacter* CrossTarget = nullptr;
        float BestCrossScore = -TNumericLimits<float>::Max();
        for (AFootballPlayerCharacter* Teammate : Teammates)
        {
            if (!IsValid(Teammate) || Teammate == BallCarrier)
            {
                continue;
            }

            const FVector TeammateLocation = Teammate->GetActorLocation();
            const float ForwardRun =
                (TeammateLocation.X - BallLocation.X) * static_cast<float>(AttackDirectionSign);
            const float Centrality = FMath::Abs(TeammateLocation.Y);
            if (ForwardRun < 150.0f || ForwardRun > 2600.0f || Centrality > 1650.0f)
            {
                continue;
            }

            const float Clearance = GetPassLaneClearance(
                BallLocation,
                TeammateLocation,
                Opponents
            );
            const float Score = ForwardRun * 0.55f +
                Clearance * 0.85f -
                Centrality * 0.32f;
            if (Score > BestCrossScore)
            {
                BestCrossScore = Score;
                CrossTarget = Teammate;
            }
        }

        const FVector CrossLocation = IsValid(CrossTarget)
            ? CrossTarget->GetActorLocation() + CrossTarget->GetVelocity() * 0.28f
            : FVector(
                BallLocation.X + static_cast<float>(AttackDirectionSign) * 1450.0f,
                0.0f,
                BallLocation.Z
            );

        if (BallCarrier->TryExecuteAITacticalKick(
            EFootballTacticalAction::Cross,
            CrossLocation
        ))
        {
            CurrentTacticalAction = EFootballTacticalAction::Cross;
            CurrentTacticalTarget = CrossTarget;
            TacticalActionCooldown = 1.75f;
            return true;
        }
    }

    AFootballPlayerCharacter* BestPassTarget = nullptr;
    EFootballTacticalAction BestPassAction = EFootballTacticalAction::None;
    float BestPassScore = -TNumericLimits<float>::Max();
    FVector BestPassLocation = FVector::ZeroVector;

    for (AFootballPlayerCharacter* Teammate : Teammates)
    {
        if (!IsValid(Teammate) || Teammate == BallCarrier)
        {
            continue;
        }

        const FVector ReceiverLocation = Teammate->GetActorLocation();
        const float Distance = FVector::Dist2D(BallLocation, ReceiverLocation);
        const float ForwardRun =
            (ReceiverLocation.X - BallLocation.X) * static_cast<float>(AttackDirectionSign);

        if (Distance < 380.0f || Distance > MaximumPassRangeCm || ForwardRun < -650.0f)
        {
            continue;
        }

        const float Clearance = GetPassLaneClearance(
            BallLocation,
            ReceiverLocation,
            Opponents
        );
        if (Clearance < MinimumPassLaneClearanceCm)
        {
            continue;
        }

        const float SupportSpacing = FVector::Dist2D(
            ReceiverLocation,
            BallCarrier->GetActorLocation()
        );
        const float Score =
            FMath::Min(Clearance, 1800.0f) * 0.72f +
            FMath::Clamp(ForwardRun, -400.0f, 1800.0f) * 0.68f -
            FMath::Abs(SupportSpacing - 1150.0f) * 0.22f;

        if (Score > BestPassScore)
        {
            BestPassScore = Score;
            BestPassTarget = Teammate;
            BestPassAction = (
                ForwardRun > 620.0f &&
                Distance > 760.0f &&
                Clearance > MinimumPassLaneClearanceCm * 1.55f
            )
                ? EFootballTacticalAction::ThroughBall
                : EFootballTacticalAction::ShortPass;
            BestPassLocation = ReceiverLocation + Teammate->GetVelocity() * (
                BestPassAction == EFootballTacticalAction::ThroughBall ? 0.52f : 0.18f
            );
        }
    }

    if (IsValid(BestPassTarget) &&
        BallCarrier->TryExecuteAITacticalKick(BestPassAction, BestPassLocation))
    {
        CurrentTacticalAction = BestPassAction;
        CurrentTacticalTarget = BestPassTarget;
        TacticalActionCooldown = 1.45f;
        return true;
    }

    CurrentTacticalAction = EFootballTacticalAction::None;
    CurrentTacticalTarget = nullptr;
    return false;
}

void AFootballTeamAIController::SetMatchPhase(const EFootballMatchPhase DesiredPhase)
{
    if (DesiredPhase == EFootballMatchPhase::TransitionToAttack ||
        DesiredPhase == EFootballMatchPhase::TransitionToDefence)
    {
        CurrentMatchPhase = DesiredPhase;
        PhaseTransitionRemaining = FMath::Max(PhaseTransitionRemaining, 0.45f);
        return;
    }

    if (DesiredPhase != LastStableMatchPhase)
    {
        CurrentMatchPhase = DesiredPhase == EFootballMatchPhase::Attacking
            ? EFootballMatchPhase::TransitionToAttack
            : EFootballMatchPhase::TransitionToDefence;
        LastStableMatchPhase = DesiredPhase;
        PhaseTransitionRemaining = 0.62f;
        return;
    }

    if (PhaseTransitionRemaining > 0.0f)
    {
        PhaseTransitionRemaining = FMath::Max(
            0.0f,
            PhaseTransitionRemaining - FMath::Max(0.05f, DecisionIntervalSeconds)
        );
        if (PhaseTransitionRemaining > 0.0f)
        {
            return;
        }
    }

    CurrentMatchPhase = DesiredPhase;
}

void AFootballTeamAIController::UpdateTeamDecision()
{
    AFootballPlayerCharacter* SelfPlayer = Cast<AFootballPlayerCharacter>(GetPawn());
    if (!IsValid(SelfPlayer) || !GetWorld())
    {
        CurrentTaskState = EFootballAITaskState::Idle;
        return;
    }

    TArray<AFootballPlayerCharacter*> AllPlayers;
    GatherFootballers(AllPlayers);

    TArray<AFootballPlayerCharacter*> Teammates;
    TArray<AFootballPlayerCharacter*> Opponents;
    const int32 TeamId = SelfPlayer->TeamId;
    const int32 AttackDirectionSign =
        SelfPlayer->AttackDirectionSign >= 0 ? 1 : -1;

    for (AFootballPlayerCharacter* Player : AllPlayers)
    {
        if (!IsValid(Player))
        {
            continue;
        }
        if (Player->TeamId == TeamId)
        {
            Teammates.Add(Player);
        }
        else
        {
            Opponents.Add(Player);
        }
    }

    AFootballBallActor* Ball = nullptr;
    float NearestBallActorDistance = TNumericLimits<float>::Max();
    for (TActorIterator<AFootballBallActor> It(GetWorld()); It; ++It)
    {
        if (!IsValid(*It))
        {
            continue;
        }
        const float Distance = FVector::Dist2D(
            SelfPlayer->GetActorLocation(),
            (*It)->GetActorLocation()
        );
        if (Distance < NearestBallActorDistance)
        {
            NearestBallActorDistance = Distance;
            Ball = *It;
        }
    }

    if (!IsValid(Ball))
    {
        SetMatchPhase(EFootballMatchPhase::TransitionToAttack);
        CurrentTacticalAction = EFootballTacticalAction::None;
        CurrentTacticalTarget = nullptr;
        CurrentTaskState = EFootballAITaskState::RecoverShape;
        MoveTowards(HomeLocation, Teammates);
        return;
    }

    const FVector BallLocation = Ball->GetActorLocation();

    AFootballMatchDirector* MatchDirector = nullptr;
    for (TActorIterator<AFootballMatchDirector> It(GetWorld()); It; ++It)
    {
        MatchDirector = *It;
        break;
    }

    if (IsValid(MatchDirector) && !MatchDirector->IsMatchActive())
    {
        CurrentTaskState = EFootballAITaskState::Idle;
        bHasCachedMoveTarget = false;
        return;
    }

    if (IsValid(MatchDirector) && MatchDirector->bRestartPending)
    {
        CurrentTacticalAction = EFootballTacticalAction::None;
        CurrentTacticalTarget = nullptr;
        if (MatchDirector->RestartTeamId == TeamId)
        {
            float NearestDistance = TNumericLimits<float>::Max();
            AFootballPlayerCharacter* RestartTaker =
                FindClosestPlayer(Teammates, BallLocation, NearestDistance);
            if (RestartTaker == SelfPlayer)
            {
                CurrentTaskState = EFootballAITaskState::RestartTaker;
                if (NearestDistance <= 360.0f && SelfPlayer->bHasBallControl)
                {
                    AFootballPlayerCharacter* Receiver = nullptr;
                    float BestProgress = -TNumericLimits<float>::Max();
                    for (AFootballPlayerCharacter* Teammate : Teammates)
                    {
                        if (!IsValid(Teammate) || Teammate == SelfPlayer) continue;
                        const float Progress =
                            (Teammate->GetActorLocation().X - BallLocation.X) *
                            static_cast<float>(AttackDirectionSign);
                        if (Progress > BestProgress) { BestProgress = Progress; Receiver = Teammate; }
                    }
                    FVector Target = IsValid(Receiver)
                        ? Receiver->GetActorLocation() + Receiver->GetVelocity() * 0.25f
                        : BallLocation + FVector(static_cast<float>(AttackDirectionSign) * 1450.0f, 0.0f, 0.0f);

                    const EFootballRestartType RestartType = MatchDirector->GetRestartType();
                    const FVector AttackingGoal(
                        static_cast<float>(AttackDirectionSign) * FieldHalfLengthCm,
                        0.0f,
                        BallLocation.Z
                    );
                    switch (RestartType)
                    {
                    case EFootballRestartType::KickOff:
                        Target = IsValid(Receiver)
                            ? Receiver->GetActorLocation() + Receiver->GetVelocity() * 0.18f
                            : BallLocation + FVector(static_cast<float>(AttackDirectionSign) * 720.0f, 0.0f, 0.0f);
                        break;
                    case EFootballRestartType::CornerKick:
                        Target = FVector(
                            AttackingGoal.X - static_cast<float>(AttackDirectionSign) * 1050.0f,
                            FMath::Clamp(-BallLocation.Y * 0.22f, -520.0f, 520.0f),
                            BallLocation.Z + 40.0f
                        );
                        break;
                    case EFootballRestartType::DirectFreeKick:
                    case EFootballRestartType::IndirectFreeKick:
                        Target = FVector::Dist2D(BallLocation, AttackingGoal) <= 2850.0f
                            ? AttackingGoal
                            : FVector(
                                AttackingGoal.X - static_cast<float>(AttackDirectionSign) * 1250.0f,
                                FMath::Clamp(BallLocation.Y * 0.18f, -450.0f, 450.0f),
                                BallLocation.Z + 30.0f
                            );
                        break;
                    case EFootballRestartType::GoalKick:
                        Target = IsValid(Receiver)
                            ? Receiver->GetActorLocation() + Receiver->GetVelocity() * 0.35f
                            : BallLocation + FVector(static_cast<float>(AttackDirectionSign) * 2200.0f, 1250.0f, 0.0f);
                        break;
                    case EFootballRestartType::ThrowIn:
                        Target = IsValid(Receiver)
                            ? Receiver->GetActorLocation() + Receiver->GetVelocity() * 0.28f
                            : BallLocation + FVector(static_cast<float>(AttackDirectionSign) * 620.0f, 0.0f, 0.0f);
                        break;
                    default:
                        break;
                    }

                    EFootballTacticalAction RestartAction = EFootballTacticalAction::ShortPass;
                    switch (MatchDirector->GetRestartType())
                    {
                    case EFootballRestartType::ThrowIn: RestartAction = EFootballTacticalAction::ThrowIn; break;
                    case EFootballRestartType::CornerKick: RestartAction = EFootballTacticalAction::CornerKick; break;
                    case EFootballRestartType::GoalKick: RestartAction = EFootballTacticalAction::GoalKick; break;
                    case EFootballRestartType::DirectFreeKick:
                    case EFootballRestartType::IndirectFreeKick: RestartAction = EFootballTacticalAction::FreeKick; break;
                    case EFootballRestartType::PenaltyKick: RestartAction = EFootballTacticalAction::PenaltyKick; break;
                    default: break;
                    }
                    if (SelfPlayer->TryExecuteAITacticalKick(RestartAction, Target))
                    {
                        CurrentTacticalAction = RestartAction;
                        CurrentTacticalTarget = Receiver;
                        bHasCachedMoveTarget = false;
                        return;
                    }
                }
                MoveTowards(BallLocation, Teammates);
            }
            else
            {
                const EFootballRestartType RestartType = MatchDirector->GetRestartType();
                if (RestartType == EFootballRestartType::CornerKick ||
                    RestartType == EFootballRestartType::DirectFreeKick ||
                    RestartType == EFootballRestartType::IndirectFreeKick)
                {
                    const float GoalX = static_cast<float>(AttackDirectionSign) * FieldHalfLengthCm;
                    const float Lane = static_cast<float>((SelfPlayer->SquadSlotIndex % 5) - 2) * 360.0f;
                    const FVector BoxRun(
                        GoalX - static_cast<float>(AttackDirectionSign) * (520.0f + (SelfPlayer->SquadSlotIndex % 3) * 260.0f),
                        Lane,
                        SelfPlayer->GetActorLocation().Z
                    );
                    CurrentTaskState = EFootballAITaskState::SupportRun;
                    MoveTowards(BoxRun, Teammates);
                }
                else
                {
                    CurrentTaskState = EFootballAITaskState::RecoverShape;
                    MoveTowards(FindFormationTarget(BallLocation, SelfPlayer, false), Teammates);
                }
            }
        }
        else
        {
            CurrentTaskState = EFootballAITaskState::RecoverShape;
            MoveTowards(FindFormationTarget(BallLocation, SelfPlayer, true), Teammates);
        }
        return;
    }

    float ClosestBallDistance = TNumericLimits<float>::Max();
    AFootballPlayerCharacter* ClosestToBall =
        FindClosestPlayer(AllPlayers, BallLocation, ClosestBallDistance);

    const bool bBallHasCarrier =
        IsValid(ClosestToBall) &&
        ClosestBallDistance <= BallPossessionRadiusCm;

    CurrentBallCarrier = bBallHasCarrier ? ClosestToBall : nullptr;

    const EFootballPlayerRole SelfRole = ResolveRole(SelfPlayer);
    if (SelfRole == EFootballPlayerRole::Goalkeeper)
    {
        const float OwnGoalX = -static_cast<float>(AttackDirectionSign) * FieldHalfLengthCm;
        const float BallDepth = (BallLocation.X - OwnGoalX) * static_cast<float>(AttackDirectionSign);
        const bool bInOwnPenaltyArea =
            BallDepth >= -150.0f && BallDepth <= 2150.0f && FMath::Abs(BallLocation.Y) <= 2350.0f;

        if (Ball->TryGoalkeeperParry(
            SelfPlayer->GetActorLocation(), TeamId, SelfPlayer, AttackDirectionSign, 175.0f
        ))
        {
            CurrentTaskState = EFootballAITaskState::GoalkeeperDive;
            bHasCachedMoveTarget = false;
            return;
        }

        if (Ball->TryGoalkeeperCatch(
            SelfPlayer->GetActorLocation(), TeamId, SelfPlayer, AttackDirectionSign, 190.0f, 1450.0f
        ))
        {
            CurrentTaskState = EFootballAITaskState::GoalkeeperCollect;
            bHasCachedMoveTarget = false;
            return;
        }

        if (Ball->IsHeldByGoalkeeper(SelfPlayer))
        {
            AFootballPlayerCharacter* Outlet = nullptr;
            float BestOutletScore = -TNumericLimits<float>::Max();
            for (AFootballPlayerCharacter* Teammate : Teammates)
            {
                if (!IsValid(Teammate) || Teammate == SelfPlayer) continue;
                const float ForwardRun =
                    (Teammate->GetActorLocation().X - SelfPlayer->GetActorLocation().X) *
                    static_cast<float>(AttackDirectionSign);
                const float WideOutlet = FMath::Abs(Teammate->GetActorLocation().Y) * 0.18f;
                const float Score = ForwardRun + WideOutlet -
                    FVector::Dist2D(SelfPlayer->GetActorLocation(), Teammate->GetActorLocation()) * 0.12f;
                if (Score > BestOutletScore)
                {
                    BestOutletScore = Score;
                    Outlet = Teammate;
                }
            }

            const FVector DistributionTarget = IsValid(Outlet)
                ? Outlet->GetActorLocation() + Outlet->GetVelocity() * 0.35f
                : SelfPlayer->GetActorLocation() +
                    FVector(static_cast<float>(AttackDirectionSign) * 2250.0f, 1150.0f, 0.0f);
            if (Ball->ReleaseFromGoalkeeper(SelfPlayer, DistributionTarget, 2350.0f, 630.0f, 110.0f))
            {
                CurrentTaskState = EFootballAITaskState::GoalkeeperDistribute;
                CurrentTacticalAction = EFootballTacticalAction::GoalKick;
                CurrentTacticalTarget = Outlet;
                if (IsValid(MatchDirector))
                {
                    MatchDirector->NotifyBallPlayed(SelfPlayer, EFootballTacticalAction::GoalKick);
                }
                bHasCachedMoveTarget = false;
                return;
            }
        }

        if (bBallHasCarrier && ClosestToBall == SelfPlayer && SelfPlayer->bHasBallControl)
        {
            AFootballPlayerCharacter* Receiver = nullptr;
            float BestProgress = -TNumericLimits<float>::Max();
            for (AFootballPlayerCharacter* Teammate : Teammates)
            {
                if (!IsValid(Teammate) || Teammate == SelfPlayer) continue;
                const float Progress =
                    (Teammate->GetActorLocation().X - BallLocation.X) * static_cast<float>(AttackDirectionSign);
                if (Progress > BestProgress) { BestProgress = Progress; Receiver = Teammate; }
            }
            const FVector Target = IsValid(Receiver)
                ? Receiver->GetActorLocation() + Receiver->GetVelocity() * 0.3f
                : BallLocation + FVector(static_cast<float>(AttackDirectionSign) * 2400.0f, 0.0f, 0.0f);
            if (SelfPlayer->TryExecuteAITacticalKick(EFootballTacticalAction::GoalKick, Target))
            {
                CurrentTaskState = EFootballAITaskState::GoalkeeperDistribute;
                CurrentTacticalAction = EFootballTacticalAction::GoalKick;
                CurrentTacticalTarget = Receiver;
                bHasCachedMoveTarget = false;
                return;
            }
        }

        const FVector KeeperTarget(
            OwnGoalX + static_cast<float>(AttackDirectionSign) * 330.0f,
            FMath::Clamp(BallLocation.Y * 0.40f, -680.0f, 680.0f),
            SelfPlayer->GetActorLocation().Z
        );
        if (bInOwnPenaltyArea && !bBallHasCarrier)
        {
            CurrentTaskState = EFootballAITaskState::GoalkeeperCollect;
            MoveTowards(BallLocation, Teammates);
        }
        else
        {
            CurrentTaskState = EFootballAITaskState::GoalkeeperSet;
            MoveTowards(KeeperTarget, Teammates);
        }
        return;
    }

    if (!bBallHasCarrier)
    {
        CurrentMatchPhase = EFootballMatchPhase::TransitionToAttack;
        CurrentTacticalAction = EFootballTacticalAction::None;
        CurrentTacticalTarget = nullptr;

        if (ClosestToBall == SelfPlayer)
        {
            CurrentTaskState = EFootballAITaskState::ChaseBall;
            MoveTowards(BallLocation, Teammates);
        }
        else
        {
            CurrentTaskState = EFootballAITaskState::RecoverShape;
            MoveTowards(
                FindFormationTarget(BallLocation, SelfPlayer, true),
                Teammates
            );
        }
        return;
    }

    const bool bOwnTeamHasPossession = ClosestToBall->TeamId == TeamId;
    if (bOwnTeamHasPossession)
    {
        SetMatchPhase(EFootballMatchPhase::Attacking);

        if (ClosestToBall == SelfPlayer)
        {
            CurrentTaskState = EFootballAITaskState::CarryBall;
            if (TryTacticalBallAction(
                SelfPlayer,
                BallLocation,
                Teammates,
                Opponents,
                AttackDirectionSign
            ))
            {
                bHasCachedMoveTarget = false;
                return;
            }

            const FVector CarryTarget = BallLocation + FVector(
                static_cast<float>(AttackDirectionSign) * 900.0f,
                -BallLocation.Y * 0.14f,
                0.0f
            );
            MoveTowards(CarryTarget, Teammates);
            return;
        }

        CurrentTacticalAction = EFootballTacticalAction::None;
        CurrentTacticalTarget = ClosestToBall;
        CurrentTaskState = EFootballAITaskState::SupportRun;
        MoveTowards(
            FindFormationTarget(BallLocation, SelfPlayer, false),
            Teammates
        );
        return;
    }

    SetMatchPhase(EFootballMatchPhase::Defending);
    CurrentTacticalAction = EFootballTacticalAction::None;
    CurrentTacticalTarget = ClosestToBall;

    // Pressing is organized by role/line: forwards and midfielders lead the press;
    // defenders step out only when the ball has entered their defensive block.
    float NearestTeammateDistance = TNumericLimits<float>::Max();
    AFootballPlayerCharacter* NearestTeammate =
        FindClosestPlayer(Teammates, BallLocation, NearestTeammateDistance);
    const EFootballPlayerRole SelfRole = ResolveRole(SelfPlayer);
    const float OwnHalfProgress =
        BallLocation.X * static_cast<float>(AttackDirectionSign);
    const bool bRoleCanLeadPress =
        SelfRole == EFootballPlayerRole::Forward ||
        SelfRole == EFootballPlayerRole::Winger ||
        SelfRole == EFootballPlayerRole::Midfielder;
    const bool bDefenderMayStepOut =
        SelfRole == EFootballPlayerRole::Defender && OwnHalfProgress < -950.0f;
    const bool bCanPress = bRoleCanLeadPress || bDefenderMayStepOut;

    if (NearestTeammate == SelfPlayer &&
        NearestTeammateDistance <= PressingRadiusCm &&
        bCanPress)
    {
        CurrentTaskState = EFootballAITaskState::PressOpponent;
        MoveTowards(ClosestToBall->GetActorLocation(), Teammates);
        ApplyCloseContest(ClosestToBall);
        return;
    }

    AFootballPlayerCharacter* MarkTarget =
        FindMarkTarget(Opponents, Teammates, BallLocation);
    if (IsValid(MarkTarget))
    {
        CurrentTaskState = EFootballAITaskState::MarkOpponent;
        const FVector GoalSideTarget = MarkTarget->GetActorLocation() - FVector(
            static_cast<float>(AttackDirectionSign) * 210.0f,
            0.0f,
            0.0f
        );
        const FVector FormationTarget =
            FindFormationTarget(BallLocation, SelfPlayer, true);
        const FVector MarkBlend = FMath::Lerp(
            GoalSideTarget,
            FormationTarget,
            SelfRole == EFootballPlayerRole::Defender ? 0.42f : 0.18f
        );
        MoveTowards(MarkBlend, Teammates);
    }
    else
    {
        CurrentTaskState = EFootballAITaskState::RecoverShape;
        MoveTowards(
            FindFormationTarget(BallLocation, SelfPlayer, true),
            Teammates
        );
    }
}

AFootballPlayerCharacter* AFootballTeamAIController::FindMarkTarget(
    const TArray<AFootballPlayerCharacter*>& OpponentPlayers,
    const TArray<AFootballPlayerCharacter*>& Teammates,
    const FVector& BallLocation
) const
{
    AFootballPlayerCharacter* SelfPlayer = Cast<AFootballPlayerCharacter>(GetPawn());
    if (!IsValid(SelfPlayer))
    {
        return nullptr;
    }

    AFootballPlayerCharacter* BestTarget = nullptr;
    float BestScore = -TNumericLimits<float>::Max();
    for (AFootballPlayerCharacter* Opponent : OpponentPlayers)
    {
        if (!IsValid(Opponent))
        {
            continue;
        }

        const float SelfDistance = FVector::Dist2D(
            SelfPlayer->GetActorLocation(),
            Opponent->GetActorLocation()
        );
        int32 CloserTeammates = 0;
        for (AFootballPlayerCharacter* Teammate : Teammates)
        {
            if (!IsValid(Teammate) || Teammate == SelfPlayer)
            {
                continue;
            }
            const float TeammateDistance = FVector::Dist2D(
                Teammate->GetActorLocation(),
                Opponent->GetActorLocation()
            );
            if (TeammateDistance + 60.0f < SelfDistance)
            {
                ++CloserTeammates;
            }
        }

        const float BallThreatDistance = FVector::Dist2D(
            Opponent->GetActorLocation(),
            BallLocation
        );
        const float Score =
            1500.0f - SelfDistance +
            FMath::Max(0.0f, 1300.0f - BallThreatDistance) * 0.28f -
            static_cast<float>(CloserTeammates) * 650.0f;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Opponent;
        }
    }
    return BestTarget;
}

void AFootballTeamAIController::MoveTowards(
    const FVector& TargetLocation,
    const TArray<AFootballPlayerCharacter*>& Teammates
)
{
    APawn* ControlledPawn = GetPawn();
    if (!IsValid(ControlledPawn))
    {
        bHasCachedMoveTarget = false;
        return;
    }

    CachedMoveTarget = TargetLocation;
    CachedTeammates.Reset();
    for (AFootballPlayerCharacter* Teammate : Teammates)
    {
        if (IsValid(Teammate))
        {
            CachedTeammates.Add(Teammate);
        }
    }

    FVector ToTarget = CachedMoveTarget - ControlledPawn->GetActorLocation();
    ToTarget.Z = 0.0f;
    bHasCachedMoveTarget = ToTarget.Size() > StopDistanceCm;
}

void AFootballTeamAIController::ApplyCachedSteering()
{
    APawn* ControlledPawn = GetPawn();
    if (!bHasCachedMoveTarget || !IsValid(ControlledPawn))
    {
        return;
    }

    FVector ToTarget = CachedMoveTarget - ControlledPawn->GetActorLocation();
    ToTarget.Z = 0.0f;
    const float Distance = ToTarget.Size();
    if (Distance <= StopDistanceCm)
    {
        bHasCachedMoveTarget = false;
        return;
    }

    FVector Steering = ToTarget.GetSafeNormal();
    FVector Separation = FVector::ZeroVector;
    for (const TWeakObjectPtr<AFootballPlayerCharacter>& TeammatePtr : CachedTeammates)
    {
        AFootballPlayerCharacter* Teammate = TeammatePtr.Get();
        if (!IsValid(Teammate) || Teammate == ControlledPawn)
        {
            continue;
        }

        FVector Away = ControlledPawn->GetActorLocation() - Teammate->GetActorLocation();
        Away.Z = 0.0f;
        const float SeparationDistance = Away.Size();
        if (SeparationDistance > 1.0f && SeparationDistance < 260.0f)
        {
            Separation += Away.GetSafeNormal() * (1.0f - SeparationDistance / 260.0f);
        }
    }

    if (!Separation.IsNearlyZero())
    {
        Steering = (Steering + Separation * 0.72f).GetSafeNormal();
    }

    const float InputScale = FMath::Clamp(
        Distance / FMath::Max(StopDistanceCm + 1.0f, SlowDownDistanceCm),
        0.18f,
        1.0f
    );
    ControlledPawn->AddMovementInput(Steering, InputScale);
}

void AFootballTeamAIController::ApplyCloseContest(AFootballPlayerCharacter* Opponent)
{
    AFootballPlayerCharacter* SelfPlayer = Cast<AFootballPlayerCharacter>(GetPawn());
    if (!bEnableAutomaticContestImpacts ||
        ContestCooldown > 0.0f ||
        !IsValid(SelfPlayer) ||
        !IsValid(Opponent))
    {
        return;
    }

    const FVector ToOpponent =
        Opponent->GetActorLocation() - SelfPlayer->GetActorLocation();
    if (ToOpponent.Size2D() > ContestDistanceCm)
    {
        return;
    }

    const FVector ImpactDirection = ToOpponent.GetSafeNormal2D();
    if (ImpactDirection.IsNearlyZero())
    {
        return;
    }

    Opponent->ApplyContestImpact(ImpactDirection, 105.0f, 0.10f);
    ContestCooldown = 1.15f;
}
