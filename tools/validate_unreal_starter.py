"""Static validation for the Unreal starter bundle.

This checks repository completeness, expected source symbols and Python builder
syntax. It does NOT compile Unreal C++; that requires Unreal Engine + MSVC on Windows.
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
    "unreal/Content/Python/build_stadium_level.py",
    "unreal/Content/Python/build_player_test_level.py",
    "unreal/Config/DefaultInput.ini",
    "tools/install_unreal_source.ps1",
    "docs/PLAYER_ENGINE_V0_2.md",
    "docs/FOOTBALL_BALL_ENGINE_V0_3.md",
]

REQUIRED_SYMBOLS = {
    "unreal/StarterSource/FootballGame/FootballBallActor.h": [
        "PhysicsSphere", "BallMassKg", "OnGoalScored", "ApplyKick", "ApplyDribbleControl",
    ],
    "unreal/StarterSource/FootballGame/FootballBallActor.cpp": [
        "SetSimulatePhysics(true)", "SetMassOverrideInKg", "RegisterGoal",
        "PitchHalfLengthCm", "AddImpulse",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerCharacter.cpp": [
        "MoveForward", "MoveRight", "StartSprint", "StopSprint",
        "AddControllerYawInput", "BrakingDecelerationWalking", "PlayerFollowCamera",
        "StartPass", "StartThroughBall", "StartShot", "ApplyDribbleControl",
    ],
    "unreal/StarterSource/FootballGame/FootballPlayerAnimInstance.h": [
        "Speed", "Direction", "VerticalSpeed", "bIsInAir", "bIsSprinting",
    ],
    "tools/install_unreal_source.ps1": [
        "FootballPlayerCharacter.cpp", "FootballPlayerAnimInstance.cpp",
        "FootballBallActor.cpp", "build_player_test_level.py", "MoveForward",
        "Gamepad_LeftY", 'ActionName="ThroughBall"', 'ActionName="Shoot"', "PhysicsCore",
    ],
    "unreal/Content/Python/build_player_test_level.py": [
        "FootballBallActor", "FPO_Ball_Test", "AutoReceiveInput.PLAYER0",
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
                failures.append(f"{relative}: Python syntax error at line {exc.lineno}: {exc.msg}")

    if failures:
        print("Unreal starter validation: FAILED")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("Unreal starter validation: OK")
    print("Ball physics, player ball actions and source presence: OK")
    print("Unreal Editor Python builder syntax: OK")
    print("Note: this is not an Unreal/MSVC C++ build.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
