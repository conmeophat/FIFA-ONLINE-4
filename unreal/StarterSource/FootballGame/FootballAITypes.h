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
