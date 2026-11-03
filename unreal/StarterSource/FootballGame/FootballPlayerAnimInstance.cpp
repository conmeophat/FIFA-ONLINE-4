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
    }
}
