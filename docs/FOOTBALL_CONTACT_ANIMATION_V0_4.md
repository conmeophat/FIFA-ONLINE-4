# Football Contact & Animation Engine v0.4

This milestone adds skeleton-aware left/right foot contact points and animation-timed ball actions to the v0.3 physics ball. It is an integration layer: the repository supplies the C++ systems and animation hooks, while the actual football animation clips and Montages must be imported from original or properly licensed assets.

## Implemented systems

- Configurable left and right contact socket names: `ball_contact_l` and `ball_contact_r` by default.
- Nearest-foot selection for pass / through-ball / shot. Dribble touches alternate left and right feet.
- Socket-aware dribble control: the physics ball receives a close-control touch only when it is within the selected foot's contact radius. If the rig has no configured sockets, a capsule-relative development fallback keeps the mannequin playable.
- Action queue: E, Q and Left Mouse Button queue a short pass, through ball or shot.
- If the matching Montage is assigned, the footballer plays it and waits for `UAnimNotify_FootballBallContact` at the actual strike frame. The notify identifies the contact foot and applies the queued action to the physics ball.
- A watchdog fallback prevents an action from being permanently lost when a montage has no notify. With no Montage assigned, a short configurable delay lets the prototype remain testable.
- Animation Blueprint variables for ball possession / dribbling, pass, through ball, shot, current action and contact foot.

## New source files

- `FootballGameplayTypes.h` — shared Blueprint enums for foot side and ball action.
- `AnimNotify_FootballBallContact.h/.cpp` — native Anim Notify that calls the owning footballer at the animation contact frame.
- Updated `FootballPlayerCharacter.h/.cpp` — foot socket queries, action queue, Montage playback, contact fallback and dribble foot alternation.
- Updated `FootballBallActor.h/.cpp` — foot-range-validated kicks and socket-position-driven dribble touch.
- Updated `FootballPlayerAnimInstance.h/.cpp` — ball-control and ball-action variables for the Animation Graph.

## Required skeleton setup

After importing a licensed Skeletal Mesh:

1. Open the Skeleton asset.
2. In Skeleton Tree, locate the left foot bone and add a Socket named `ball_contact_l`.
3. Locate the right foot bone and add a Socket named `ball_contact_r`.
4. Move each socket toward the boot instep / toe where the ball should be contacted. Keep the socket name exactly as configured on the player actor, or change the two socket-name properties in its Details panel.
5. Save the Skeleton.

The C++ code calls the Skeletal Mesh component's socket transform at runtime. If a socket does not exist, it uses a simple character-relative position intended only for the prototype mannequin.

## Create action Montages and add contact notifies

Create compatible animations and Montages using the same Skeleton as the player:

1. Import one short pass animation, one through-ball animation and one shot animation.
2. Create an Animation Montage for each animation.
3. Open each Montage. At the precise frame where the boot should strike the ball, add a Notify.
4. Choose **Football Ball Contact** (the native `UAnimNotify_FootballBallContact` class).
5. Set Contact Foot to Left, Right or Auto. Auto uses the nearest foot selected when the action was queued.
6. In the player actor's Details panel, assign the three assets to **Pass Montage**, **Through Ball Montage** and **Shot Montage**.

When the key is pressed, the matching Montage starts. The ball impulse is deferred until the notify event arrives, so the visible boot strike and ball launch can be aligned. If a notify is missing, a watchdog eventually performs the action as a safety net; this is not as precise as a correctly placed notify.

## Animation Blueprint setup

The C++ parent `FootballPlayerAnimInstance` now exposes:

| Variable | Purpose |
|---|---|
| `Speed`, `Direction`, `VerticalSpeed`, `bIsInAir` | Locomotion |
| `bIsSprinting` | Sprint state |
| `bHasBallControl` | Ball is within the player's close-control area |
| `bIsDribbling` | Player is moving while controlling the ball |
| `bIsPassing` | Short-pass action is active |
| `bIsThroughBall` | Through-ball action is active |
| `bIsShooting` | Shot action is active |
| `bIsUsingLeftFoot` | Currently selected contact foot |
| `BallAction` | Enum: None / ShortPass / ThroughBall / Shot |
| `ContactFoot` | Enum: Auto / Left / Right |

Recommended Animation Graph wiring:

- Keep the normal locomotion Blend Space for movement without the ball.
- Add a dribble locomotion Blend Space or State Machine branch and select it while `bIsDribbling` is true. Use Speed / Direction to drive the blend.
- Add a Slot node (for example `DefaultGroup.DefaultSlot`) in the final pose path so the action Montages can play over the locomotion graph.
- Use `bIsPassing`, `bIsThroughBall` and `bIsShooting` for action-specific upper-body / recovery states if required. The Montage remains the timing source for the kick frame.
- Use `bIsUsingLeftFoot` or `ContactFoot` to select foot-specific poses or contact effects.

The project does not include third-party animation assets. The user must import compatible clips and place the notify at the correct strike frame.

## Run the updated prototype

From the repository root in PowerShell:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\\tools\\install_unreal_source.ps1
~~~

Then regenerate project files, build **Development Editor** in Visual Studio, reopen Unreal, and run `Content/Python/build_player_test_level.py` in the saved stadium map. Press Play and approach the spawned ball.

- WASD: move
- Mouse: rotate the camera
- Left Shift: sprint
- E: short pass
- Q: through ball
- Left Mouse Button: shot

Without imported animations and assigned Montages, movement and fallback ball actions still work, but there will be no realistic footballer kick/dribble animation. The engine can only become visually synchronized after the compatible assets, sockets and notify frames are configured.

## Smoke test checklist

- The imported rig exposes both `ball_contact_l` and `ball_contact_r` sockets.
- A slowly moving player near the ball triggers alternating left/right close-control touches.
- Pressing E / Q / Left Mouse starts the corresponding Montage.
- The ball stays under physics before the notify, then accelerates from the selected foot at the notify frame.
- The left/right notify option selects the correct contact socket; Auto uses the queued nearest foot.
- Animation Blueprint state variables change during dribbling and each ball action.
- A missing notify is recovered by the watchdog fallback rather than losing the queued input.

## Limitations

- No animation FBX, Montages, rigged player model or foot-contact VFX are bundled; users must supply licensed assets.
- The v0.4 socket touch is a proximity-gated physics velocity assist, not a fully simulated deformable boot / ball collision solver.
- The prototype still has no opponent or teammate AI, goalkeepers, tackle / interception, complete match rules, match clock, scoreboard HUD or replay.
- GitHub Actions validates file presence, required symbols and Python syntax and runs the Python backend tests. It does not compile Unreal C++ or verify actual animation playback. Build and test inside the target Unreal Engine + MSVC installation.
