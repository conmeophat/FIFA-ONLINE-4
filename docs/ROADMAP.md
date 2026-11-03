# Development roadmap

## Milestone 0 — Repository foundation
- [x] Python API skeleton and validated domain models
- [x] Synthetic fixture data labelled as fictional
- [x] API tests and CI
- [x] Windows setup guide and local backend runner
- [ ] Local developer environment verified on the target PC
- [ ] Unreal C++ project generated and checked in (with Git LFS for binary assets)

## Milestone 1 — Stadium and broadcast camera blockout
- [x] Scale-aware pitch surface and football markings
- [x] Penalty areas, goal frames and corner flags
- [x] Three-tier stand blockout and floodlight masts
- [x] Smooth high sideline broadcast camera with tracked actor and pitch clamps
- [x] Unreal Editor Python script to place actors in the level
- [x] PowerShell installer for the stadium/camera source
- [ ] Compile the C++ project on target Windows / Unreal installation
- [ ] Verify the level visually and assign project materials / lighting

## Milestone 2 — Football Player Engine v0.2
- [x] Controllable C++ ACharacter footballer
- [x] Camera-relative WASD and gamepad-left-stick movement
- [x] Follow camera with mouse/right-stick look
- [x] Sprint, braking and prototype jump
- [x] Temporary primitive mannequin proxy
- [x] C++ Animation Blueprint parent exposing locomotion variables
- [x] Editor Python script to spawn and possess the test character
- [x] Installer merges input mappings and copies player source
- [ ] Compile the player module on target Windows / Unreal installation
- [ ] Possess and move the player in the local stadium level
- [ ] Import a licensed rigged footballer model and configure its Animation Blueprint
- [ ] Verify locomotion Blend Space and tune speed / turn / braking

## Milestone 3 — Football Ball Engine v0.3
- [x] Physics-driven ball actor with collision sphere, mass, friction/restitution and damping
- [x] Proximity-based assisted close control / dribble steering
- [x] Short pass, through ball and lifted shot actions
- [x] Goal-line crossing detection, score counters and Blueprint goal event
- [x] Editor Python test-level builder spawns one player and one ball
- [x] v0.4: left/right foot sockets, nearest-foot selection and alternating dribble touches
- [x] v0.4: Anim Notify defers pass / through-ball / shot impulse to the strike frame
- [x] v0.4: Animation Blueprint variables expose dribbling, action and contact-foot state
- [ ] Import licensed football animations, configure sockets / Montages and verify timing in Unreal
- [ ] Match clock, scoreboard UI and restart states
- [ ] Football locomotion animations (jog, sprint, turn, stop, dribble)

## Milestone 4 — Foot Contact & Animation Engine v0.4
- [x] Configurable left/right ball-contact socket names with prototype fallback positions
- [x] Alternating left/right dribble contact and nearest-foot action selection
- [x] Native Anim Notify forwards the kick-frame contact event to the footballer
- [x] Pass / through-ball / shot Montages are configurable per player actor
- [x] Animation Blueprint exposes ball-control and ball-action state variables
- [ ] Import licensed character animation clips, create Montages and place strike notifies
- [ ] Tune foot socket placement and verify animation timing in local Unreal playtests

## Milestone 5 — Football Player Control Engine v0.5
- [x] Explicit body facing: rotate toward movement input and turn toward a nearby ball when slowing down
- [x] CharacterMovement acceleration, braking and friction tuning for controllable inertia
- [x] Direction-change speed penalty and sprint-turn-rate limits
- [x] Balance model driven by lateral acceleration and sharp sprint turns
- [x] Brace input reduces contest knockback / balance loss and slows movement
- [x] Blueprint-callable contest impact API for future opponent / tackle overlap systems
- [x] Animation Blueprint variables for acceleration, turn angle, balance and contest reaction
- [ ] Connect opponent / teammate overlap events to ApplyContestImpact in a later AI milestone
- [ ] Tune all movement values in local Unreal playtests with the imported rig and locomotion animations

## Milestone 6 — Offline match slice
- [ ] Add goalkeeper and simple teammate/opponent AI
- [ ] Build restart states (kick-off, goal, throw-in, corner, free kick)
- [ ] Add substitution and match summary
- [ ] Profile CPU/GPU and validate stable frame pacing

## Milestone 7 — Football management
- [ ] Connect Unreal to the Python catalogue API
- [ ] Add squads, formations, player positions and team tactics
- [ ] Add local saves and career progression
- [ ] Introduce a licensed, documented real-world data provider only after legal and schema review

## Milestone 8 — Online foundation
- [ ] Design account/session services and an authoritative match server
- [ ] Implement matchmaking and reconnect handling
- [ ] Add server-side validation, rate limits, logging and anti-cheat controls
- [ ] Load test before public deployment

Each milestone should be independently runnable and tested. Do not claim 11v11, official worldwide rosters, online play or FC Online-level fidelity until those features exist and have been verified.
