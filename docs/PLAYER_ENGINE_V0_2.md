# Football Player Engine v0.2 — Controllable Player Prototype

This milestone adds a controllable Unreal C++ footballer, a temporary primitive mannequin, camera-relative movement, sprint/braking, gamepad mappings and an Animation Blueprint parent class.

**Scope:** the player can be possessed and moved for a prototype test. This is not yet a football-specific animation set: there are no dribble, pass, shoot, tackle or ball-contact animations in this milestone.

## Source files

Copy/installation is handled by \`tools/install_unreal_source.ps1\`. The following sources are copied into the generated Unreal Runtime module:

- \`FootballPlayerCharacter.h/.cpp\` — pawn, camera, movement and input.
- \`FootballPlayerAnimInstance.h/.cpp\` — Speed, Direction, VerticalSpeed, bIsInAir, bIsSprinting and bHasMovementInput variables for Animation Blueprints.

The installer also copies \`build_player_test_level.py\` and merges the keyboard/gamepad mappings into the project's \`Config/DefaultInput.ini\`.

## 1. Install and compile

From repository root in PowerShell, close Unreal Editor and run:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\install_unreal_source.ps1
~~~

The script:
- Finds the Unreal \`.uproject\` recursively under \`unreal/\`.
- Copies the six stadium/camera/API files plus the four player/animation C++ files into the project's Runtime module.
- Rewrites the API export macro and Python reflected class paths to match the actual module name.
- Ensures HTTP, Json and JsonUtilities Build.cs dependencies exist.
- Adds player key/gamepad mappings without replacing unrelated existing input settings.
- Copies both Editor Python builder scripts.

Then regenerate project files and build the **Development Editor** target in Visual Studio. Reopen the project after a successful local build.

## 2. Create a test player in the level

1. Open the stadium blockout map.
2. Run **Tools → Execute Python Script**.
3. Run \`Content/Python/build_player_test_level.py\`.
4. The script spawns \`FPO_Player_Test\` near the centre of the pitch and sets Auto Possess Player 0.
5. It disables automatic activation of the stadium broadcast camera for this player-control test. The broadcast camera actor remains in the level and can be activated later for spectator presentation.
6. Press Play.

The character is initially a simple engine-built block mannequin assembled from built-in cube, sphere and cylinder meshes. This placeholder verifies that possession, movement and camera work before importing a real rigged character.

## 3. Controls

| Input | Action |
|---|---|
| W / S | Move forward / backward relative to camera yaw |
| A / D | Move left / right relative to camera yaw |
| Mouse | Rotate follow camera |
| Left Shift | Sprint while held |
| Space | Prototype jump |
| Gamepad left stick | Move |
| Gamepad right stick | Rotate camera |
| Gamepad left shoulder | Sprint |
| Gamepad bottom face button | Prototype jump |

The player rotates toward the movement direction. Sprint changes MaxWalkSpeed; braking deceleration and ground friction control how the player slows down. These are exposed as editable values on the character actor.

The default development tuning is:
- Walk speed: 480 cm/s (4.8 m/s).
- Sprint multiplier: 1.55 (about 7.44 m/s).
- Acceleration: 2400 cm/s².
- Braking deceleration: 2200 cm/s².
- Character capsule: 38 cm radius, 96 cm half-height.

These are prototype values and should be tuned after playtesting.

## 4. Import a rigged footballer model

Use a model you created or have a license to use in this game. A marketplace model's availability for download does not automatically grant permission to redistribute its mesh, textures, animations, likeness or kit in this repository.

1. Import the FBX into Unreal as a **Skeletal Mesh**.
2. Import or select the matching Skeleton.
3. Check the model scale, forward axis, root bone and bind pose in the import preview.
4. Import compatible locomotion animations using the same Skeleton, or retarget them with Unreal's IK Rig / IK Retargeter.
5. Select the placed \`FPO_Player_Test\` actor.
6. In **Football → Appearance**, set:
   - \`Player Skeletal Mesh\` to the imported Skeletal Mesh.
   - \`Player Animation Blueprint\` to the Animation Blueprint described below.
7. Adjust \`Skeletal Mesh Relative Location\` and \`Skeletal Mesh Relative Rotation\` if your model's origin or facing axis differs.

When a Skeletal Mesh is assigned, the primitive placeholder is hidden automatically. When no mesh is assigned, the placeholder remains visible.

## 5. Create an Animation Blueprint

The C++ class \`UFootballPlayerAnimInstance\` is the parent for a project Animation Blueprint.

1. In Content Browser, right-click the imported Skeleton.
2. Choose **Create → Animation Blueprint**.
3. Set Parent Class to \`FootballPlayerAnimInstance\` (search the class name in the parent-class picker).
4. In the Animation Blueprint, the following variables are updated from C++:
   - \`Speed\`: horizontal speed in cm/s.
   - \`Direction\`: movement direction relative to character facing, in degrees.
   - \`VerticalSpeed\`: vertical velocity.
   - \`bIsInAir\`: falling state.
   - \`bIsSprinting\`: sprint input state.
   - \`bHasMovementInput\`: whether movement input is present.
5. Create a locomotion Blend Space (1D on Speed, or 2D on Direction and Speed) using only animations compatible with the imported Skeleton.
6. Add a locomotion state to the AnimGraph and connect the Blend Space player output to the Final Animation Pose.
7. Add jump/fall states based on \`bIsInAir\` when suitable animations exist.
8. Compile and save the Animation Blueprint, then assign it to the character's \`Player Animation Blueprint\` property.

Do not use animations from a different skeleton unless they are properly retargeted first.

## 6. How the camera works

The character owns a Spring Arm and Follow Camera:
- The camera follows behind the controlled player.
- Mouse/right-stick input changes controller yaw/pitch.
- Movement uses only controller yaw, so W always means "forward relative to the camera" and D means "right relative to the camera".
- Camera lag smooths movement and rotation.
- The existing stadium broadcast camera remains separate for future match spectating, replay and broadcast presentation.

## 7. Smoke test

- Press Play; the placed footballer is possessed by Player 0.
- W moves in the direction the camera faces.
- A/D move laterally relative to the camera.
- Rotating the mouse changes the forward travel direction.
- Holding Left Shift increases top speed; releasing it restores walk speed.
- Releasing movement input causes the character to decelerate.
- A compatible Skeletal Mesh + Animation Blueprint replaces the primitive placeholder.

If the player does not move, verify that the installer ran after the C++ project was generated, the Development Editor target compiled, and the level contains \`FPO_Player_Test\` with Auto Possess Player 0. Also check that \`Config/DefaultInput.ini\` contains MoveForward, MoveRight, Turn, LookUp, Sprint and Jump mappings.

## Known limitations

- The included mannequin is geometric placeholder art, not a real player likeness or production character.
- No commercial player model, Real Madrid kit, club crest or proprietary animation is bundled.
- The camera is a third-person development camera for direct player control. The broadcast camera remains a separate actor.
- Ball dribbling, touches, passing, shooting, tackling, player locomotion animation assets and football AI are later milestones.
- Unreal C++ compilation and gameplay verification must occur on the local Windows/Unreal/MSVC installation; GitHub Actions currently tests the Python backend only.
