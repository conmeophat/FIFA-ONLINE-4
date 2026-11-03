#include "FootballPlayerAnimInstance.h"

#include "FootballPlayerCharacter.h"
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
        bHasBallControl = false;
        bIsDribbling = false;
        bIsBallActionActive = false;
        bIsPassing = false;
        bIsThroughBall = false;
        bIsShooting = false;
        bIsUsingLeftFoot = false;
        BallAction = EFootballBallAction::None;
        ContactFoot = EFootballContactFoot::Right;
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
        }
    }

    if (const AFootballPlayerCharacter* Footballer = Cast<AFootballPlayerCharacter>(Pawn))
    {
        bIsSprinting = Footballer->bIsSprinting;
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
}
