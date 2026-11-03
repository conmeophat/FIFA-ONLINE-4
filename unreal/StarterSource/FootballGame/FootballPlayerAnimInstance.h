#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FootballGameplayTypes.h"
#include "FootballAITypes.h"
#include "FootballPlayerAnimInstance.generated.h"

/**
 * Animation Blueprint parent for the football player.
 * Exposes locomotion, ball-control and ball-action state to the Animation Graph.
 */
UCLASS(Blueprintable, Transient)
class FOOTBALLGAME_API UFootballPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    float VerticalSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    bool bHasMovementInput = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Movement")
    float MovementAcceleration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Movement")
    float TurnAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Movement")
    bool bIsChangingDirection = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Balance")
    float Balance = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Balance")
    bool bIsOffBalance = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Balance")
    bool bIsBracing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Balance")
    bool bIsContestStunned = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Contest")
    bool bIsInShoulderDuel = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Contest")
    float ContestReactionStrength = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Balance")
    EFootballBalanceState BalanceState = EFootballBalanceState::Stable;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Control")
    bool bHasBallControl = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Control")
    bool bIsDribbling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    bool bIsBallActionActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    bool bIsPassing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    bool bIsThroughBall = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    bool bIsShooting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    bool bIsUsingLeftFoot = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    EFootballBallAction BallAction = EFootballBallAction::None;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Ball Action")
    EFootballContactFoot ContactFoot = EFootballContactFoot::Right;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Animation")
    EFootballAnimationCue AnimationCue = EFootballAnimationCue::None;

    UPROPERTY(BlueprintReadOnly, Category = "Football|AI")
    bool bIsAIControlled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Football|AI")
    EFootballAITaskState AITaskState = EFootballAITaskState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Tactics")
    EFootballMatchPhase MatchPhase = EFootballMatchPhase::TransitionToAttack;

    UPROPERTY(BlueprintReadOnly, Category = "Football|Tactics")
    EFootballTacticalAction TacticalAction = EFootballTacticalAction::None;
};
