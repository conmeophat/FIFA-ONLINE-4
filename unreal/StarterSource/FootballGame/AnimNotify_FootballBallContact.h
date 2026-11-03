#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FootballGameplayTypes.h"
#include "AnimNotify_FootballBallContact.generated.h"

/**
 * Place this notify on the exact kick frame of a pass / through-ball / shot
 * montage. It forwards the selected contact foot to the owning footballer.
 */
UCLASS(meta = (DisplayName = "Football Ball Contact"))
class FOOTBALLGAME_API UAnimNotify_FootballBallContact : public UAnimNotify
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Contact")
    EFootballContactFoot ContactFoot = EFootballContactFoot::Auto;

    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
    ) override;
};
