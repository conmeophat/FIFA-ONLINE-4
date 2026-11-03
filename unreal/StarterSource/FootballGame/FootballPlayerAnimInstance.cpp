#include "FootballPlayerAnimInstance.h"

#include "FootballPlayerCharacter.h"
#include "FootballTeamAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UFootballPlayerAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* Pawn = TryGetPawnOwner();
    if (!IsValid(Pawn))
    {
        Speed = 0.0f;
        Direction = 0.0f;
        VerticalSpeed = 0.0f;
        bIsInAir = false;
        bIsSprinting = false;
        bHasMovementInput = false;
        MovementAcceleration = 0.0f;
        TurnAngle = 0.0f;
        bIsChangingDirection = false;
        Balance = 1.0f;
        bIsOffBalance = false;
        bIsBracing = false;
        bIsContestStunned = false;
        BalanceState = EFootballBalanceState::Stable;
        bHasBallControl = false;
        bIsDribbling = false;
        bIsBallActionActive = false;
        bIsPassing = false;
        bIsThroughBall = false;
        bIsShooting = false;
        bIsUsingLeftFoot = false;
        BallAction = EFootballBallAction::None;
        ContactFoot = EFootballContactFoot::Right;
        bIsAIControlled = false;
        AITaskState = EFootballAITaskState::Idle;
        MatchPhase = EFootballMatchPhase::TransitionToAttack;
        TacticalAction = EFootballTacticalAction::None;
        return;
    }

    const FVector Velocity = Pawn->GetVelocity();
    Speed = Velocity.Size2D();
    VerticalSpeed = Velocity.Z;
    bHasMovementInput = !Pawn->GetLastMovementInputVector().IsNearlyZero();

    if (Speed > 1.0f)
    {
        Direction = FMath::FindDeltaAngleDegrees(
            Pawn->GetActorRotation().Yaw,
            Velocity.ToOrientationRotator().Yaw
        );
    }
    else
    {
        Direction = 0.0f;
    }

    if (const ACharacter* Character = Cast<ACharacter>(Pawn))
    {
        if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            bIsInAir = Movement->IsFalling();
            MovementAcceleration = Movement->GetCurrentAcceleration().Size2D();
        }
    }

    if (const AFootballPlayerCharacter* Footballer = Cast<AFootballPlayerCharacter>(Pawn))
    {
        bIsSprinting = Footballer->bIsSprinting;
        bIsChangingDirection = Footballer->bIsChangingDirection;
        Balance = Footballer->Balance;
        bIsOffBalance = Footballer->bIsOffBalance;
        bIsBracing = Footballer->bIsBracing;
        bIsContestStunned = Footballer->bIsContestStunned;
        BalanceState = Footballer->BalanceState;
        TurnAngle = Footballer->GetMovementDirection();
        bHasBallControl = Footballer->bHasBallControl;
        bIsDribbling = Footballer->bIsDribbling;
        BallAction = Footballer->ActiveBallAction;
        ContactFoot = Footballer->ActiveContactFoot;

        bIsBallActionActive = BallAction != EFootballBallAction::None;
        bIsPassing = BallAction == EFootballBallAction::ShortPass;
        bIsThroughBall = BallAction == EFootballBallAction::ThroughBall;
        bIsShooting = BallAction == EFootballBallAction::Shot;
        bIsUsingLeftFoot = ContactFoot == EFootballContactFoot::Left;
    }

    if (const AFootballTeamAIController* AIController = Cast<AFootballTeamAIController>(Pawn->GetController()))
    {
        bIsAIControlled = true;
        AITaskState = AIController->GetCurrentTaskState();
        MatchPhase = AIController->GetCurrentMatchPhase();
        TacticalAction = AIController->GetCurrentTacticalAction();
    }
    else
    {
        bIsAIControlled = false;
        AITaskState = EFootballAITaskState::Idle;
        MatchPhase = EFootballMatchPhase::TransitionToAttack;
        TacticalAction = EFootballTacticalAction::None;
    }
}
