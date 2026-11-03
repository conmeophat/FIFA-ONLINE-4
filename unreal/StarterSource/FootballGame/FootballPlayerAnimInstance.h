#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FootballPlayerAnimInstance.generated.h"

/**
 * Animation Blueprint parent for the football player.
 * Exposes locomotion values to the Animation Graph without per-frame Blueprint casts.
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
};
