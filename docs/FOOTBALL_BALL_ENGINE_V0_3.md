# Football Ball Engine v0.3 — Physics Ball and Basic Football Actions

Milestone v0.3 puts one controllable player and one physics-driven football on the stadium blockout. It is a playable gameplay slice, not yet a complete match simulation.

## What is implemented

- A football actor with a spherical physics collision body, a visible engine-sphere mesh, an approximate 0.43 kg mass, friction/restitution override, damping and gravity.
- Player proximity-based close control. While the ball is in front and within control range, the player gently steers its horizontal velocity toward a point in front of the character. The ball remains a simulated rigid body and still collides with the pitch.
- Three kick actions: a low short pass, a slightly lifted through ball and a lifted shot. The impulses are mass-independent and have separate speed, lift and spin tuning.
- Goal-line crossing detection for both ends of the 105 m pitch. A goal is counted only when the ball crosses the X-plane inside the 7.32 m goal mouth and below the 2.44 m crossbar. The ball exposes a Blueprint event with the goal side and both score counters, then resets at the centre after 1.5 seconds.
- An Editor Python builder that spawns the player and one ball in the existing stadium level.
- Keyboard and common gamepad mappings installed by the PowerShell source installer.

## Source files

- `unreal/StarterSource/FootballGame/FootballBallActor.h/.cpp` — collision sphere, physical material, kick impulses, close-control assist and goal detection.
- `FootballPlayerCharacter.h/.cpp` — proximity ball acquisition, dribble steering and pass/through-ball/shot inputs.
- `unreal/Content/Python/build_player_test_level.py` — places the player and ball into the active map.
- `unreal/Config/DefaultInput.ini` — input mappings.
- `tools/install_unreal_source.ps1` — copies the complete source set into the generated Unreal Runtime module and merges input mappings.

## Controls

| Input | Action |
|---|---|
| W / A / S / D | Move relative to camera yaw |
| Mouse | Rotate player follow camera |
| Left Shift | Sprint |
| Space | Prototype jump |
| E | Short ground pass |
| Q | Through ball |
| Left Mouse Button | Shoot |
| Gamepad left stick | Move |
| Gamepad right stick | Camera |
| Gamepad left shoulder | Sprint |
| Gamepad face button bottom | Jump |
| Gamepad face button left (X) | Pass |
| Gamepad face button top (Y) | Through ball |
| Gamepad face button right (B) | Shoot |

Kick direction follows the player's facing direction. Move/turn the player before kicking to change the direction.

## Install and run on Windows

1. Generate a C++ Blank Unreal project named `FootballGame` somewhere under the repository's `unreal/` directory.
2. Enable **Python Editor Script Plugin** and **Editor Scripting Utilities**.
3. Close Unreal Editor.
4. From the repository root, run:

~~~powershell
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

5. Regenerate project files and build the **Development Editor** target in Visual Studio.
6. Reopen Unreal, open the saved stadium map and run **Tools → Execute Python Script**.
7. Execute `Content/Python/build_player_test_level.py`.
8. Press Play. Move the player close to the ball, then press E, Q or Left Mouse Button.

The builder removes its previous `FPO_Player_Test` and `FPO_Ball_Test` actors before spawning replacements, so it can be rerun safely.

## Tuning

Initial physical prototype values:
- Ball radius: 11 cm.
- Ball mass: approximately 0.43 kg.
- Friction coefficient: 0.55.
- Restitution: 0.52.
- Linear damping: 0.16.
- Angular damping: 0.32.
- Close-control range: about 2.8 m.
- Short pass: 1,500 cm/s impulse change.
- Through ball: 1,900 cm/s with 65 cm/s lift.
- Shot: 2,550 cm/s with 420 cm/s lift.

These are starting values for testing, not measured professional-match ball parameters.

## Smoke test

- The level contains one player and one visible ball.
- Walking into the ball's control radius brings it toward a point in front of the player.
- E sends a low pass; Q sends a lifted through ball; Left Mouse Button sends a faster shot.
- The ball rolls and bounces under physics instead of being attached to the player.
- A shot that crosses either goal plane inside the posts and under the crossbar increments the corresponding score counter and broadcasts `OnGoalScored`.
- The ball returns to the centre after a goal.

## Important limitations

- Close control is a physics-velocity assist based on player proximity, not yet literal animated foot-to-ball contact. A future animation/socket system should create timed touches from left/right foot sockets.
- No goalkeeper, defenders, teammates, player switching, tackle, interception, throw-in, corner, free kick, penalty, offside, match clock, scoreboard UI, replay or full 11v11 simulation is included.
- The goal event and counters are available to Blueprint, but no HUD has been built yet.
- The pitch and player remain blockout assets. Use original or properly licensed models, animations, kits, sounds and likenesses.
- GitHub CI only performs static source checks plus Python/backend tests. It does not compile Unreal C++ or simulate physics. The generated project must be built with the matching Unreal Engine and MSVC toolchain on Windows and play-tested locally.
