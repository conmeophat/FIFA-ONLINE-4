#include "AnimNotify_FootballBallContact.h"

#include "Components/SkeletalMeshComponent.h"
#include "FootballPlayerCharacter.h"

void UAnimNotify_FootballBallContact::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference
)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!IsValid(MeshComp))
    {
        return;
    }

    AFootballPlayerCharacter* Footballer =
        Cast<AFootballPlayerCharacter>(MeshComp->GetOwner());

    if (IsValid(Footballer))
    {
        Footballer->HandleBallContactNotify(ContactFoot);
    }
}
