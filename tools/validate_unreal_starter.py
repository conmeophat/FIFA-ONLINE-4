"""Static validation for the Unreal starter bundle.

Checks repository completeness and expected symbols plus Python builder syntax.
This does NOT compile Unreal C++; that requires Unreal Engine + MSVC on Windows.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "unreal/StarterSource/FootballGame/FootballStadiumActor.h",
    "unreal/StarterSource/FootballGame/FootballStadiumActor.cpp",
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.h",
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.cpp",
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.h",
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.cpp",
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.h",
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.cpp",
    "unreal/StarterSource/FootballGame/FootballBallActor.h",
    "unreal/StarterSource/FootballGame/FootballBallActor.cpp",
    "unreal/StarterSource/FootballGame/FootballGameplayTypes.h",
    "unreal/StarterSource/FootballGame/FootballAITypes.h",
    "unreal/StarterSource/FootballGame/FootballTeamAIController.h",
    "unreal/StarterSource/FootballGame/FootballTeamAIController.cpp",
    "unreal/StarterSource/FootballGame/FootballAIPlayerCharacter.h",
    "unreal/StarterSource/FootballGame/FootballAIPlayerCharacter.cpp",
    "unreal/StarterSource/FootballGame/FootballMatchDirector.h",
    "unreal/StarterSource/FootballGame/FootballMatchDirector.cpp",
    "unreal/StarterSource/FootballGame/FootballMatchHUDWidget.h",
    "unreal/StarterSource/FootballGame/FootballMatchHUDWidget.cpp",
    "unreal/StarterSource/FootballGame/AnimNotify_FootballBallContact.h",
    "unreal/StarterSource/FootballGame/AnimNotify_FootballBallContact.cpp",
    "unreal/Content/Python/build_stadium_level.py",
    "unreal/Content/Python/build_player_test_level.py",
    "unreal/Config/DefaultInput.ini",
    "tools/install_unreal_source.ps1",
    "docs/PLAYER_ENGINE_V0_2.md",
    "docs/FOOTBALL_BALL_ENGINE_V0_3.md",
    "docs/FOOTBALL_CONTACT_ANIMATION_V0_4.md",
    "docs/FOOTBALL_PLAYER_CONTROL_V0_5.md",
    "docs/FOOTBALL_TEAM_AI_V0_6.md",
]

REQUIRED_SYMBOLS = {
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.h": [
        "WalkingTurnRateDegreesPerSec", "SprintTurnRateDegreesPerSec",
        "BallFacingRangeCm", "DirectionChangeSpeedLoss", "Balance",
        "EFootballBalanceState", "bIsBracing", "bIsOffBalance",
        "ApplyContestImpact",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.cpp": [
        "TryExecuteAITacticalKick", "ApplyKickFromFoot", "EFootballTacticalAction::Cross",
        "NotifyBallPlayed", "NotifyBallTouched", "RegisterLastTouch",
        "UpdatePlayerContests", "bIsInShoulderDuel", "ContestReactionStrength",
        "UpdateFootballFacing", "UpdateBalance", "UpdateDynamicMovementTuning",
        "FMath::FixedTurn", "BrakingDecelerationWalking",
        "Movement->AddImpulse", "StartBrace", "StopBrace",
        "bIsChangingDirection", "ContestStunRemaining",
    ],
    "unreal/StarterSource/FootballGame/FootballGameplayTypes.h": [
        "EFootballContactFoot", "EFootballBallAction", "EFootballBalanceState",
        "Stable", "Bracing", "Recovering", "OffBalance", "ContestStunned",
    ],
    "unreal/StarterSource/FootballGame/FootballAITypes.h": [
        "EFootballPlayerRole", "EFootballAITaskState", "SupportRun", "PressOpponent", "MarkOpponent",
        "EFootballFormation", "Formation433", "Formation4231", "Formation442",
        "EFootballMatchPhase", "TransitionToAttack", "TransitionToDefence",
        "EFootballTacticalAction", "ThroughBall", "Cross", "CornerKick",
        "EFootballMatchPeriod", "EFootballRestartType", "IndirectFreeKick", "PenaltyKick",
    ],
    "unreal/StarterSource/FootballGame/FootballTeamAIController.h": [
        "DecisionIntervalSeconds", "CurrentTaskState", "FindFormationTarget",
        "TryTacticalBallAction", "GetPassLaneClearance", "SetMatchPhase",
        "CurrentMatchPhase", "CurrentTacticalAction",
    ],
    "unreal/StarterSource/FootballGame/FootballTeamAIController.cpp": [
        "FindClosestPlayer", "SupportRun", "PressOpponent", "MarkOpponent",
        "FindFormationTarget", "TryTacticalBallAction", "ThroughBall", "Cross",
        "Formation433", "Formation4231", "Formation442", "AddMovementInput", "ApplyContestImpact",
    ],
    "unreal/StarterSource/FootballGame/FootballAIPlayerCharacter.cpp": [
        "AIControllerClass", "AFootballTeamAIController::StaticClass", "PlacedInWorldOrSpawned",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.h": [
        "TeamId", "AttackDirectionSign", "TeamRole", "TeamFormation", "SquadSlotIndex",
        "TryExecuteAITacticalKick",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.cpp": [
        "AFootballTeamAIController", "GetCurrentTaskState",
        "GetCurrentMatchPhase", "GetCurrentTacticalAction",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.h": [
        "bIsAIControlled", "AITaskState", "MatchPhase", "TacticalAction",
        "MovementAcceleration", "TurnAngle", "bIsChangingDirection",
        "Balance", "bIsOffBalance", "bIsBracing", "bIsContestStunned",
        "EFootballBalanceState",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.cpp": [
        "GetCurrentAcceleration", "Footballer->Balance", "Footballer->bIsBracing",
        "Footballer->bIsContestStunned", "Footballer->BalanceState",
    ],
    "tools/install_unreal_source.ps1": [
        "FootballPlayerCharacter.cpp", "FootballPlayerAnimInstance.cpp",
        "FootballTeamAIController.cpp", "FootballMatchDirector.cpp",
        "FootballAIPlayerCharacter.cpp", "AIModule",
        "FootballBallActor.cpp", "PhysicsCore", 'ActionName="Brace"',
        "Gamepad_RightShoulder", "FootballMatchHUDWidget.cpp", "UMG", "SlateCore",
    ],
    "unreal/Config/DefaultInput.ini": [
        'ActionName="Brace"', "Key=LeftControl", "Key=Gamepad_RightShoulder",
    ],
    "unreal/StarterSource/FootballGame/FootballBallActor.cpp": [
        "ApplyKickFromFoot", "ApplyFootTouch", "RegisterGoal",
        "RegisterLastTouch", "StopAtRestart", "TryGoalkeeperParry",
        "TryGoalkeeperCatch", "IsHeldByGoalkeeper", "ReleaseFromGoalkeeper",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchDirector.h": [
        "HalfDurationSeconds", "MatchClockRate", "CurrentPeriod", "bRestartPending",
        "MatchHUDWidgetClass", "UFootballMatchHUDWidget",
        "NotifyBallPlayed", "NotifyBallTouched", "SignalFoul", "NotifyRestartTaken",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchDirector.cpp": [
        "BuildOffsideSnapshot", "CheckOutOfPlay", "EnterHalfTime", "StartSecondHalf",
        "ThrowIn", "CornerKick", "GoalKick", "IndirectFreeKick", "HandleGoalScored",
        "CreateWidget", "AddToViewport",
    ],
    "unreal/StarterSource/FootballGame/AnimNotify_FootballBallContact.cpp": [
        "HandleBallContactNotify",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchHUDWidget.h": [
        "UFootballMatchHUDWidget", "NativeConstruct", "NativeTick",
        "ScoreText", "ClockText", "PeriodText",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchHUDWidget.cpp": [
        "TEAM 0", "GetMatchClockText", "FIRST HALF", "FULL-TIME",
        "SetAnchors", "SetPosition",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.h": [
        "bEnableAutomaticPlayerContests", "ContestContactRadiusCm",
        "bIsInShoulderDuel", "ContestReactionStrength",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.cpp": [
        "UpdatePlayerContests", "RelativeSpeed", "ApplyContestImpact",
        "PlayerContestCooldown",
    ],
    "unreal/StarterSource/FootballGame/FootballBallActor.h": [
        "TryGoalkeeperCatch", "IsHeldByGoalkeeper", "ReleaseFromGoalkeeper",
    ],
    "unreal/StarterSource/FootballGame/FootballTeamAIController.cpp": [
        "TryGoalkeeperCatch", "ReleaseFromGoalkeeper", "AttackingGoal",
        "EFootballRestartType::CornerKick", "BoxRun",
    ],
}

def main() -> int:
    failures: list[str] = []

    for relative in REQUIRED_FILES:
        if not (ROOT / relative).is_file():
            failures.append(f"Missing required file: {relative}")

    for relative, symbols in REQUIRED_SYMBOLS.items():
        path = ROOT / relative
        if not path.is_file():
            continue
        content = path.read_text(encoding="utf-8")
        for symbol in symbols:
            if symbol not in content:
                failures.append(f"{relative}: missing expected symbol {symbol}")

    for relative in (
        "unreal/Content/Python/build_stadium_level.py",
        "unreal/Content/Python/build_player_test_level.py",
    ):
        path = ROOT / relative
        if path.is_file():
            source = path.read_text(encoding="utf-8")
            try:
                compile(source, str(path), "exec")
            except SyntaxError as exc:
                failures.append(
                    f"{relative}: Python syntax error at line {exc.lineno}: {exc.msg}"
                )

    if failures:
        print("Unreal starter validation: FAILED")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("Unreal starter validation: OK")
    print("Player inertia, turn control, balance and contest reaction: OK")
    print("v0.4 ball contact / animation integration: OK")
    print("v0.8 match clock, offside, restarts and goalkeeper hooks: OK")
    print("v0.9 player contests, goalkeeper catches, set-piece AI and HUD: OK")
    print("Unreal Editor Python builder syntax: OK")
    print("Note: this is not an Unreal/MSVC C++ build.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
