# Football Player Control Engine v0.5 — Inertia, Facing, Balance and Contest Response

This milestone upgrades the v0.4 footballer from a basic third-person pawn to a football-specific movement controller. It uses Unreal's CharacterMovementComponent for acceleration / braking and adds explicit facing, turn penalties, balance state and a contest-response API.

## Implemented

### 1. Body facing and turning toward the ball
- CharacterMovement no longer automatically rotates the body to movement direction.
- While movement input exists, the body turns toward the camera-relative move direction.
- If the player is slowing down and a tracked ball is within the configurable facing radius, the body turns toward the ball.
- If there is no input and the ball is not in the facing radius, the body follows the current velocity direction.
- Rotation uses a fixed degrees-per-second turn rate, with separate walking and sprinting values. Balance and brace state modify turn speed.

### 2. Acceleration, braking and inertia
- CharacterMovement keeps velocity between input changes instead of snapping to the new direction.
- Max acceleration, braking deceleration, ground friction and separate braking friction are applied as exposed tuning values.
- During a sharp direction change, the maximum speed is temporarily reduced based on the dot product between current velocity and requested movement direction.
- Sprinting has a lower turn rate and a stronger balance penalty on rapid cuts.

### 3. Balance model
- Balance is a normalized value from 0.0 to 1.0.
- Lateral acceleration consumes balance, especially at high speed and while sprinting.
- Releasing hard movement lets balance recover gradually.
- Balance affects top speed, acceleration and body turning.
- The states are Stable, Bracing, Recovering, OffBalance and ContestStunned.

### 4. Bracing and contest reaction
- Hold Left Control or Gamepad Right Shoulder to brace.
- Bracing slows movement and acceleration, but reduces incoming contest impulse and balance damage.
- ApplyContestImpact(Direction, ImpulseCmPerSec, BalanceDamage) is BlueprintCallable so future opponent / tackle / shoulder-challenge overlap logic can invoke a consistent response.
- The impact impulse is applied through CharacterMovementComponent AddImpulse, and a short contest-stun state temporarily restricts speed and turning.

The current milestone supplies the response mechanism; it does not yet include opponent AI or automatically detect tackles.

## Controls

| Input | Action |
|---|---|
| W / A / S / D | Move relative to camera yaw |
| Mouse | Rotate follow camera |
| Left Shift | Sprint |
| Space | Prototype jump |
| E | Short pass |
| Q | Through ball |
| Left Mouse Button | Shoot |
| Left Control (hold) | Brace / resist a challenge |
| Gamepad right shoulder (hold) | Brace |
| Gamepad left shoulder | Sprint |

## Blueprint / Animation Blueprint variables

The parent FootballPlayerAnimInstance now exposes:

| Variable | Meaning |
|---|---|
| MovementAcceleration | Current horizontal acceleration magnitude |
| TurnAngle | Movement direction relative to the character facing |
| bIsChangingDirection | Requested movement direction diverges from current velocity |
| Balance | Normalized balance from 0 to 1 |
| bIsOffBalance | Balance has dropped below the safe threshold |
| bIsBracing | Brace input is held |
| bIsContestStunned | A recent contest impact is still affecting control |
| BalanceState | Stable / Bracing / Recovering / OffBalance / ContestStunned |

Recommended Animation Graph usage:
- Use MovementAcceleration and TurnAngle to blend start / stop / turn locomotion.
- Use bIsChangingDirection to blend a plant-and-cut animation.
- Use Balance to blend stumble / recovery poses.
- Use bIsBracing to select a shield / brace stance.
- Use bIsContestStunned and BalanceState to select hit reaction and balance-recovery states.

## Calling the contest API

An opponent Blueprint, tackle trace or overlap can call:

~~~text
ApplyContestImpact(ImpactDirection, ImpulseCmPerSec, BalanceDamage)
~~~

- ImpactDirection: horizontal push direction, usually from the challenger toward the player.
- ImpulseCmPerSec: velocity change added to the character, clamped by the C++ implementation.
- BalanceDamage: normalized balance amount removed, from 0.0 to 0.8.
- Holding brace reduces both effects. The response is also exposed to Animation Blueprint state variables.

## Install on Windows

From the repository root:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

Regenerate project files, build Development Editor in Visual Studio, open the saved stadium level and run Content/Python/build_player_test_level.py. Press Play and test movement, sharp changes of direction, releasing the controls, looking at the nearby ball, and holding Left Control during a contest impact triggered from Blueprint.

## Smoke test checklist

- Body turns toward WASD direction instead of being driven by the default OrientRotationToMovement.
- Releasing movement input lets the character decelerate through CharacterMovement braking.
- A sharp 180-degree direction change is slower than maintaining a straight line.
- Sprint turns are more deliberate and reduce balance under a hard cut.
- Balance gradually recovers after high lateral acceleration stops.
- Holding Left Control reduces incoming contest shove and balance loss.
- Calling ApplyContestImpact adds a controlled horizontal push and briefly enters ContestStunned.
- Animation Blueprint variables reflect turn, acceleration, balance and brace state.

## Limitations

- No opponent AI or automatic tackle / overlap event is included yet. An AI / tackle system must call ApplyContestImpact.
- Balance is a gameplay tuning model, not a full biomechanical centre-of-mass or foot-plant simulation.
- The body turn and balance states need imported locomotion, turn, stumble, brace and recovery animations to become visually realistic.
- Unreal C++ compilation and game feel require the local Unreal Engine + MSVC environment. GitHub CI only checks source presence / expected symbols, Python syntax and the existing backend tests.
