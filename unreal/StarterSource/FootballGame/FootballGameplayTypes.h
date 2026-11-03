#pragma once

#include "CoreMinimal.h"
#include "FootballGameplayTypes.generated.h"

UENUM(BlueprintType)
enum class EFootballContactFoot : uint8
{
    Auto UMETA(DisplayName = "Automatic / nearest foot"),
    Left UMETA(DisplayName = "Left foot"),
    Right UMETA(DisplayName = "Right foot")
};

UENUM(BlueprintType)
enum class EFootballBallAction : uint8
{
    None UMETA(DisplayName = "None"),
    ShortPass UMETA(DisplayName = "Short pass"),
    ThroughBall UMETA(DisplayName = "Through ball"),
    Shot UMETA(DisplayName = "Shot")
};
