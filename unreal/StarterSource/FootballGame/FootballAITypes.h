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
    RecoverShape UMETA(DisplayName = "Recover team shape"),
    GoalkeeperSet UMETA(DisplayName = "Goalkeeper set position"),
    GoalkeeperDive UMETA(DisplayName = "Goalkeeper save"),
    GoalkeeperCollect UMETA(DisplayName = "Goalkeeper collect"),
    GoalkeeperDistribute UMETA(DisplayName = "Goalkeeper distribute"),
    RestartTaker UMETA(DisplayName = "Take restart")
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
    Shot UMETA(DisplayName = "Shot"),
    ThrowIn UMETA(DisplayName = "Throw-in"),
    CornerKick UMETA(DisplayName = "Corner kick"),
    GoalKick UMETA(DisplayName = "Goal kick"),
    FreeKick UMETA(DisplayName = "Free kick"),
    PenaltyKick UMETA(DisplayName = "Penalty kick")
};

UENUM(BlueprintType)
enum class EFootballMatchPeriod : uint8
{
    PreMatch UMETA(DisplayName = "Pre-match"),
    FirstHalf UMETA(DisplayName = "First half"),
    HalfTime UMETA(DisplayName = "Half-time"),
    SecondHalf UMETA(DisplayName = "Second half"),
    FullTime UMETA(DisplayName = "Full-time")
};

UENUM(BlueprintType)
enum class EFootballRestartType : uint8
{
    None UMETA(DisplayName = "None"),
    KickOff UMETA(DisplayName = "Kick-off"),
    ThrowIn UMETA(DisplayName = "Throw-in"),
    CornerKick UMETA(DisplayName = "Corner kick"),
    GoalKick UMETA(DisplayName = "Goal kick"),
    DirectFreeKick UMETA(DisplayName = "Direct free kick"),
    IndirectFreeKick UMETA(DisplayName = "Indirect free kick"),
    PenaltyKick UMETA(DisplayName = "Penalty kick")
};
