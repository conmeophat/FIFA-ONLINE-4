#pragma once

#include "CoreMinimal.h"
#include "FootballPlayerCharacter.h"
#include "FootballAIPlayerCharacter.generated.h"

/** Testable AI footballer that automatically creates the team AI controller. */
UCLASS(Blueprintable)
class FOOTBALLGAME_API AFootballAIPlayerCharacter : public AFootballPlayerCharacter
{
    GENERATED_BODY()

public:
    AFootballAIPlayerCharacter();
};
