#include "FootballTeamAIController.h"

#include "FootballBallActor.h"
#include "FootballPlayerCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

void AFootballTeamAIController::OnUnPossess()
{
    CurrentBallCarrier = nullptr;
    CurrentTaskState = EFootballAITaskState::Idle;
    bHasCachedMoveTarget = false;
    CachedTeammates.Reset();
    Super::OnUnPossess();
}

void AFootballTeamAIController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ContestCooldown = FMath::Max(0.0f, ContestCooldown - DeltaSeconds);
    DecisionAccumulator += DeltaSeconds;
    if (DecisionAccumulator >= FMath::Max(0.05f, DecisionIntervalSeconds))
    {
        DecisionAccumulator = 0.0f;
        UpdateTeamDecision();
    }

    // Movement input must be supplied every frame; tactical decisions are throttled.
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
    const int32 AttackDirectionSign = SelfPlayer->AttackDirectionSign >= 0 ? 1 : -1;
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
        const float Distance = FVector::Dist2D(SelfPlayer->GetActorLocation(), (*It)->GetActorLocation());
        if (Distance < NearestBallActorDistance)
        {
            NearestBallActorDistance = Distance;
            Ball = *It;
        }
    }

    if (!IsValid(Ball))
    {
        CurrentTaskState = EFootballAITaskState::RecoverShape;
        MoveTowards(HomeLocation, Teammates);
        return;
    }

    const FVector BallLocation = Ball->GetActorLocation();
    float ClosestBallDistance = TNumericLimits<float>::Max();
    AFootballPlayerCharacter* ClosestToBall =
        FindClosestPlayer(AllPlayers, BallLocation, ClosestBallDistance);

    const bool bBallHasCarrier =
        IsValid(ClosestToBall) && ClosestBallDistance <= BallPossessionRadiusCm;
    CurrentBallCarrier = bBallHasCarrier ? ClosestToBall : nullptr;

    if (!bBallHasCarrier)
    {
        // A loose ball is contested by the nearest footballer, not by the whole squad.
        if (ClosestToBall == SelfPlayer)
        {
            CurrentTaskState = EFootballAITaskState::ChaseBall;
            MoveTowards(BallLocation, Teammates);
        }
        else
        {
            CurrentTaskState = EFootballAITaskState::RecoverShape;
            const FVector ShapeTarget = HomeLocation + FVector(
                static_cast<float>(AttackDirectionSign) * 180.0f, 0.0f, 0.0f
            );
            MoveTowards(ShapeTarget, Teammates);
        }
        return;
    }

    const bool bOwnTeamHasPossession = ClosestToBall->TeamId == TeamId;
    if (bOwnTeamHasPossession)
    {
        if (ClosestToBall == SelfPlayer)
        {
            CurrentTaskState = EFootballAITaskState::CarryBall;
            const FVector GoalRun = BallLocation + FVector(
                static_cast<float>(AttackDirectionSign) * 1150.0f,
                -BallLocation.Y * 0.22f,
                0.0f
            );
            MoveTowards(GoalRun, Teammates);
            return;
        }

        CurrentTaskState = EFootballAITaskState::SupportRun;
        const FVector Space = FindSpaceTarget(
            BallLocation, Teammates, Opponents, AttackDirectionSign, SelfPlayer->TeamRole
        );
        MoveTowards(Space, Teammates);
        return;
    }

    // One player presses; the rest protect lanes and mark opponents.
    float NearestTeammateDistance = TNumericLimits<float>::Max();
    AFootballPlayerCharacter* NearestTeammate =
        FindClosestPlayer(Teammates, BallLocation, NearestTeammateDistance);

    if (NearestTeammate == SelfPlayer && NearestTeammateDistance <= PressingRadiusCm)
    {
        CurrentTaskState = EFootballAITaskState::PressOpponent;
        MoveTowards(ClosestToBall->GetActorLocation(), Teammates);
        ApplyCloseContest(ClosestToBall);
        return;
    }

    AFootballPlayerCharacter* MarkTarget = FindMarkTarget(Opponents, Teammates, BallLocation);
    if (IsValid(MarkTarget))
    {
        CurrentTaskState = EFootballAITaskState::MarkOpponent;
        const FVector GoalSideTarget = MarkTarget->GetActorLocation() - FVector(
            static_cast<float>(AttackDirectionSign) * 210.0f, 0.0f, 0.0f
        );
        MoveTowards(GoalSideTarget, Teammates);
    }
    else
    {
        CurrentTaskState = EFootballAITaskState::RecoverShape;
        MoveTowards(HomeLocation, Teammates);
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

        const float SelfDistance =
            FVector::Dist2D(SelfPlayer->GetActorLocation(), Opponent->GetActorLocation());
        int32 CloserTeammates = 0;
        for (AFootballPlayerCharacter* Teammate : Teammates)
        {
            if (!IsValid(Teammate) || Teammate == SelfPlayer)
            {
                continue;
            }
            const float TeammateDistance =
                FVector::Dist2D(Teammate->GetActorLocation(), Opponent->GetActorLocation());
            if (TeammateDistance + 60.0f < SelfDistance)
            {
                ++CloserTeammates;
            }
        }

        const float BallThreatDistance =
            FVector::Dist2D(Opponent->GetActorLocation(), BallLocation);
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

FVector AFootballTeamAIController::FindSpaceTarget(
    const FVector& BallLocation,
    const TArray<AFootballPlayerCharacter*>& Teammates,
    const TArray<AFootballPlayerCharacter*>& Opponents,
    const int32 AttackDirectionSign,
    const EFootballPlayerRole Role
) const
{
    AFootballPlayerCharacter* SelfPlayer = Cast<AFootballPlayerCharacter>(GetPawn());
    if (!IsValid(SelfPlayer))
    {
        return BallLocation;
    }

    float ForwardOffset = 350.0f;
    switch (Role)
    {
    case EFootballPlayerRole::Goalkeeper:
        ForwardOffset = -3600.0f;
        break;
    case EFootballPlayerRole::Defender:
        ForwardOffset = -500.0f;
        break;
    case EFootballPlayerRole::Winger:
        ForwardOffset = 850.0f;
        break;
    case EFootballPlayerRole::Forward:
        ForwardOffset = 1250.0f;
        break;
    case EFootballPlayerRole::Midfielder:
    default:
        ForwardOffset = 350.0f;
        break;
    }

    FVector Anchor = BallLocation + FVector(
        static_cast<float>(AttackDirectionSign) * ForwardOffset,
        HomeLocation.Y * 0.28f,
        0.0f
    );
    if (Role == EFootballPlayerRole::Goalkeeper)
    {
        Anchor = HomeLocation;
        Anchor.Y = FMath::Clamp(BallLocation.Y * 0.35f, -650.0f, 650.0f);
    }

    const FVector CandidateOffsets[] = {
        FVector(-360.0f, -780.0f, 0.0f),
        FVector(0.0f, -480.0f, 0.0f),
        FVector(360.0f, -780.0f, 0.0f),
        FVector(-420.0f, 0.0f, 0.0f),
        FVector(0.0f, 0.0f, 0.0f),
        FVector(420.0f, 0.0f, 0.0f),
        FVector(-360.0f, 780.0f, 0.0f),
        FVector(0.0f, 480.0f, 0.0f),
        FVector(360.0f, 780.0f, 0.0f)
    };

    FVector BestLocation = Anchor;
    float BestScore = -TNumericLimits<float>::Max();
    for (const FVector& Offset : CandidateOffsets)
    {
        FVector Candidate = Anchor + FVector(
            Offset.X * static_cast<float>(AttackDirectionSign), Offset.Y, 0.0f
        );
        Candidate.X = FMath::Clamp(
            Candidate.X, -FieldHalfLengthCm + 250.0f, FieldHalfLengthCm - 250.0f
        );
        Candidate.Y = FMath::Clamp(
            Candidate.Y, -FieldHalfWidthCm + 250.0f, FieldHalfWidthCm - 250.0f
        );
        Candidate.Z = SelfPlayer->GetActorLocation().Z;

        float NearestOpponent = 1500.0f;
        for (AFootballPlayerCharacter* Opponent : Opponents)
        {
            if (IsValid(Opponent))
            {
                NearestOpponent = FMath::Min(
                    NearestOpponent, FVector::Dist2D(Candidate, Opponent->GetActorLocation())
                );
            }
        }

        float NearestTeammate = 1300.0f;
        for (AFootballPlayerCharacter* Teammate : Teammates)
        {
            if (IsValid(Teammate) && Teammate != SelfPlayer)
            {
                NearestTeammate = FMath::Min(
                    NearestTeammate, FVector::Dist2D(Candidate, Teammate->GetActorLocation())
                );
            }
        }

        const float AnchorDeviation = FVector::Dist2D(Candidate, Anchor);
        const float HomeDeviation = FVector::Dist2D(Candidate, HomeLocation);
        const float ForwardProgress =
            (Candidate.X - BallLocation.X) * static_cast<float>(AttackDirectionSign);
        const float Score =
            FMath::Min(NearestOpponent, 1500.0f) * 0.0018f +
            FMath::Min(NearestTeammate, 1300.0f) * 0.0012f -
            AnchorDeviation * 0.0010f -
            HomeDeviation * (Role == EFootballPlayerRole::Goalkeeper ? 0.0015f : 0.00025f) +
            ForwardProgress * 0.00012f;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestLocation = Candidate;
        }
    }
    return BestLocation;
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
        Distance / FMath::Max(StopDistanceCm + 1.0f, SlowDownDistanceCm), 0.18f, 1.0f
    );
    ControlledPawn->AddMovementInput(Steering, InputScale);
}

void AFootballTeamAIController::ApplyCloseContest(AFootballPlayerCharacter* Opponent)
{
    AFootballPlayerCharacter* SelfPlayer = Cast<AFootballPlayerCharacter>(GetPawn());
    if (!bEnableAutomaticContestImpacts || ContestCooldown > 0.0f ||
        !IsValid(SelfPlayer) || !IsValid(Opponent))
    {
        return;
    }

    const FVector ToOpponent = Opponent->GetActorLocation() - SelfPlayer->GetActorLocation();
    if (ToOpponent.Size2D() > ContestDistanceCm)
    {
        return;
    }

    const FVector ImpactDirection = ToOpponent.GetSafeNormal2D();
    if (ImpactDirection.IsNearlyZero())
    {
        return;
    }

    // Use a restrained shoulder-contact impulse with the v0.5 balance / brace system.
    Opponent->ApplyContestImpact(ImpactDirection, 105.0f, 0.10f);
    ContestCooldown = 1.15f;
}
