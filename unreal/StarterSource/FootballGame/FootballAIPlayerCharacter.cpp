#include "FootballAIPlayerCharacter.h"

#include "FootballTeamAIController.h"

AFootballAIPlayerCharacter::AFootballAIPlayerCharacter()
{
    AIControllerClass = AFootballTeamAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
