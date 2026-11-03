"""Static validation for the Unreal starter bundle.

Checks source bundle completeness, key v0.2-v1.0 systems and Editor Python syntax.
This does NOT compile Unreal C++; use Unreal Engine + MSVC on Windows for that.
"""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "unreal/StarterSource/FootballGame/FootballStadiumActor.h",
    "unreal/StarterSource/FootballGame/FootballStadiumActor.cpp",
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.h",
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.cpp",
    "unreal/StarterSource/FootballGame/FootballRefereeActor.h",
    "unreal/StarterSource/FootballGame/FootballRefereeActor.cpp",
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
    "docs/FOOTBALL_MATCH_ENGINE_V0_9.md",
    "docs/FOOTBALL_MATCH_ENGINE_V1_0.md",
]

REQUIRED_SYMBOLS = {
    "unreal/StarterSource/FootballGame/FootballGameplayTypes.h": [
        "EFootballContactFoot", "EFootballBallAction", "EFootballBalanceState",
        "EFootballAnimationCue", "GoalkeeperCatch", "GoalkeeperDive",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.h": [
        "TeamId", "AttackDirectionSign", "TeamRole", "TeamFormation", "SquadSlotIndex",
        "TryExecuteAITacticalKick", "ApplyContestImpact",
        "bEnableAutomaticPlayerContests", "ContestContactRadiusCm",
        "bIsInShoulderDuel", "ContestReactionStrength", "ActiveAnimationCue",
        "ShoulderDuelMontage", "FoulReactionMontage", "GoalkeeperCatchMontage",
        "GoalkeeperDiveMontage", "GoalkeeperDistributionMontage",
        "PlayShoulderDuelMontage", "PlayFoulReactionMontage",
        "PlayGoalkeeperCatchMontage", "PlayGoalkeeperDiveMontage",
        "PlayGoalkeeperDistributionMontage",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.cpp": [
        "TryExecuteAITacticalKick", "ApplyKickFromFoot", "UpdatePlayerContests",
        "ReportContestContact", "PlayConfiguredMontage", "ClearAnimationCue",
        "ApplyContestImpact", "FMath::FixedTurn", "UpdateDynamicMovementTuning",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.h": [
        "Speed", "Direction", "Balance", "AITaskState", "MatchPhase",
        "bIsInShoulderDuel", "ContestReactionStrength", "AnimationCue",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.cpp": [
        "GetCurrentAcceleration", "Footballer->Balance",
        "Footballer->ActiveAnimationCue", "GetCurrentTaskState",
    ],
    "unreal/StarterSource/FootballGame/FootballBallActor.h": [
        "TryGoalkeeperCatch", "IsHeldByGoalkeeper", "ReleaseFromGoalkeeper",
    ],
    "unreal/StarterSource/FootballGame/FootballBallActor.cpp": [
        "TryGoalkeeperCatch", "IsHeldByGoalkeeper", "ReleaseFromGoalkeeper",
        "AttachToComponent", "DetachFromActor",
    ],
    "unreal/StarterSource/FootballGame/FootballTeamAIController.cpp": [
        "FindClosestPlayer", "TryTacticalBallAction", "TryGoalkeeperCatch",
        "ReleaseFromGoalkeeper", "PlayGoalkeeperCatchMontage",
        "PlayGoalkeeperDiveMontage", "PlayGoalkeeperDistributionMontage",
        "EFootballRestartType::CornerKick", "BoxRun",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchDirector.h": [
        "HalfDurationSeconds", "MatchClockRate", "CurrentPeriod",
        "MatchHUDWidgetClass", "ReportContestContact", "RequestSubstitution",
        "MaxSubstitutionsPerTeam", "TeamZeroSubstitutionsUsed",
        "TeamOneSubstitutionsUsed", "OnSubstitutionMade",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchDirector.cpp": [
        "BuildOffsideSnapshot", "CheckOutOfPlay", "SignalFoul",
        "AutomaticFoulSpeedThresholdCmPerSec", "ReportContestContact",
        "RequestSubstitution", "SpawnActor<AFootballPlayerCharacter>",
        "OnSubstitutionMade.Broadcast", "CreateWidget", "AddToViewport",
    ],
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.h": [
        "TrackedActor", "bActivateAsPlayerViewOnBeginPlay",
        "bHasActivatedPlayerView",
    ],
    "unreal/StarterSource/FootballGame/FootballBroadcastCameraActor.cpp": [
        "SetViewTargetWithBlend", "GetFocusLocation", "GetDesiredCameraLocation",
    ],
    "unreal/StarterSource/FootballGame/FootballRefereeActor.h": [
        "AFootballRefereeActor", "TrackedBall", "FollowOffsetCm", "bAutoTrackBall",
    ],
    "unreal/StarterSource/FootballGame/FootballRefereeActor.cpp": [
        "FindTrackedBall", "VInterpTo", "RInterpTo", "NoCollision",
    ],
    "unreal/StarterSource/FootballGame/FootballMatchHUDWidget.cpp": [
        "TEAM 0", "GetMatchClockText", "FIRST HALF", "FULL-TIME",
    ],
    "tools/install_unreal_source.ps1": [
        "FootballRefereeActor.h", "FootballRefereeActor.cpp",
        "FootballBroadcastCameraActor.cpp", "FootballMatchHUDWidget.cpp",
        "UMG", "Slate", "SlateCore", "v1.0",
    ],
    "unreal/Content/Python/build_player_test_level.py": [
        "FootballRefereeActor", "FootballBroadcastCameraActor",
        "FPO_Broadcast_Camera", "FPO_Referee",
        "b_activate_as_player_view_on_begin_play", "tracked_ball",
        "full 11v11",
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
        source = path.read_text(encoding="utf-8")
        for symbol in symbols:
            if symbol not in source:
                failures.append(f"{relative}: missing expected symbol {symbol}")

    for relative in (
        "unreal/Content/Python/build_stadium_level.py",
        "unreal/Content/Python/build_player_test_level.py",
    ):
        path = ROOT / relative
        if path.is_file():
            try:
                compile(path.read_text(encoding="utf-8"), str(path), "exec")
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
    print("Player controls, contacts and Animation Blueprint cues: OK")
    print("v0.8-v0.9 match rules, goalkeepers, set pieces and HUD: OK")
    print("v1.0 automatic foul heuristics and substitutions: OK")
    print("v1.0 broadcast camera and referee actor: OK")
    print("Unreal Editor Python builder syntax: OK")
    print("Note: this is not an Unreal/MSVC C++ build or runtime playtest.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
