#pragma once

#include "CoreMinimal.h"
#include "FootballAITypes.generated.h"

UENUM(BlueprintType)
enum class EFootballPlayerRole : uint8
{
    Goalkeeper UMETA(DisplayName = "Goalkeeper"),
    Defender UMETA(DisplayName = "Defender"),
    Midfielder UMETA(DisplayName = "Midfielder"),
    Winger UMETA(DisplayName = "Winger"),
    Forward UMETA(DisplayName = "Forward")
};

UENUM(BlueprintType)
enum class EFootballAITaskState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    ChaseBall UMETA(DisplayName = "Chase ball"),
    CarryBall UMETA(DisplayName = "Carry ball"),
    SupportRun UMETA(DisplayName = "Support run"),
    FindSpace UMETA(DisplayName = "Find space"),
    PressOpponent UMETA(DisplayName = "Press opponent"),
    MarkOpponent UMETA(DisplayName = "Mark opponent"),
    RecoverShape UMETA(DisplayName = "Recover team shape")
};


UENUM(BlueprintType)
enum class EFootballFormation : uint8
{
    Formation433 UMETA(DisplayName = "4-3-3"),
    Formation4231 UMETA(DisplayName = "4-2-3-1"),
    Formation442 UMETA(DisplayName = "4-4-2")
};

UENUM(BlueprintType)
enum class EFootballMatchPhase : uint8
{
    TransitionToAttack UMETA(DisplayName = "Transition to attack"),
    Attacking UMETA(DisplayName = "Attacking"),
    TransitionToDefence UMETA(DisplayName = "Transition to defence"),
    Defending UMETA(DisplayName = "Defending")
};

UENUM(BlueprintType)
enum class EFootballTacticalAction : uint8
{
    None UMETA(DisplayName = "None"),
    ShortPass UMETA(DisplayName = "Short pass"),
    ThroughBall UMETA(DisplayName = "Through ball"),
    Cross UMETA(DisplayName = "Cross"),
    Clear UMETA(DisplayName = "Clear"),
    Shot UMETA(DisplayName = "Shot")
};
