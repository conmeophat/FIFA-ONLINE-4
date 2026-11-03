# FIFA ONLINE 4 — Python + Unreal Football Project

> An independent football game project inspired by online football management and match simulations. It is not affiliated with EA or the original FIFA / FC Online products.

## Technology direction

- **Game client:** Unreal Engine 5; C++ and Blueprints handle packaged-game runtime gameplay.
- **Python:** FastAPI backend, football-data validation/import, management rules, Editor automation and developer tools.
- **First target:** a playable offline prototype with a broadcast-style 3D camera.
- **Later target:** online sessions after the offline match simulation is stable.

Unreal's Python integration is an Editor scripting environment, not the runtime gameplay language for packaged games. Player input, movement, animation, collision, ball physics and match simulation belong in Unreal C++/Blueprints. Python provides data, APIs and tooling. See the [technical architecture](docs/ARCHITECTURE.md).

## Repository layout

~~~text
.
├── backend/          # FastAPI service, domain models and tests
├── data/fixtures/    # Synthetic development data only
├── docs/             # Architecture, roadmap and Windows guides
├── tools/            # Backend and Unreal source installer scripts
├── unreal/           # C++ actors, player, Editor Python builders and input config
└── .github/          # GitHub Actions CI
~~~

## Unreal prototype milestones

### Stadium 3D + broadcast camera
- Scale-aware pitch, markings, penalty areas, goals, flags, stand blockout and floodlight masts.
- High sideline broadcast camera actor with smooth tracking and pitch bounds.
- Unreal Editor Python script to place the actors in the current level.

See [Stadium 3D + Broadcast Camera](docs/STADIUM_BROADCAST_CAMERA.md).

### Football Player Engine v0.2
- C++ controllable footballer character with camera-relative movement.
- Follow camera, mouse/gamepad look, sprint, braking and prototype jump.
- Primitive mannequin placeholder so the player is visible without importing external art.
- C++ Animation Blueprint parent exposing Speed, Direction, VerticalSpeed, bIsInAir and bIsSprinting.
- Editor Python script to spawn and auto-possess a test player.
- PowerShell installer that copies all C++ starter files, adds missing module dependencies and merges input mappings.

See [Football Player Engine v0.2](docs/PLAYER_ENGINE_V0_2.md).

### Football Ball Engine v0.3
- Physics-driven football actor with a spherical collision body, mass, friction/restitution and damping.
- Assisted close control while the player is near and facing the ball.
- Short pass (E), through ball (Q) and shot (Left Mouse Button), plus gamepad equivalents.
- Goal-line detection at both ends, score counters and a Blueprint event.
- Test-level builder spawns the player and ball together.

See [Football Ball Engine v0.3](docs/FOOTBALL_BALL_ENGINE_V0_3.md).

### Football Contact & Animation Engine v0.4
- Configurable left/right foot-contact sockets and alternating dribble touches.
- Short pass, through-ball and shot Montages with a native Anim Notify at the actual contact frame.
- Ball impulse is triggered by the notify, not immediately when the input key is pressed.
- Animation Blueprint exposes dribbling, ball action, and selected-foot variables.
- Watchdog fallback keeps the action playable if a Montage notify was omitted.

See [Football Contact & Animation Engine v0.4](docs/FOOTBALL_CONTACT_ANIMATION_V0_4.md).

### Football Player Control Engine v0.5
- Explicit body rotation toward the movement direction and nearby ball.
- CharacterMovement acceleration / braking inertia, turn-rate limits and reduced speed during sharp changes of direction.
- Balance meter reacts to lateral acceleration and sharp sprint turns.
- Brace input reduces contest impact and balance damage at the cost of movement speed.
- Blueprint-callable contest impact hook for future tackles / shoulder challenges.
- Animation Blueprint variables expose acceleration, turning, balance and contest state.

See [Football Player Control Engine v0.5](docs/FOOTBALL_PLAYER_CONTROL_V0_5.md).

### Football Team AI Engine v0.6
- AI-controlled footballer pawn automatically creates its team AI controller.
- Team-aware loose-ball chase and ball-carrier recognition.
- Off-ball support runs and open-space selection that scores opponent distance, teammate spacing and field position.
- Coordinated defending: one closest teammate presses while others mark opponents and protect goal-side lanes.
- Close shoulder-contest invokes the v0.5 balance / brace response with a cooldown.
- Animation Blueprint exposes AI-controlled state and current task for state-machine blending.
- Editor test builder spawns one human player and seven AI footballers across two teams.

See [Football Team AI Engine v0.6](docs/FOOTBALL_TEAM_AI_V0_6.md).

### Football Tactical AI Engine v0.7
- Team formation presets: 4-3-3, 4-2-3-1 and 4-4-2, with squad-slot coordinates.
- Shared attacking / defending phase state and transition state exposed to Animation Blueprint.
- Role- and line-aware pressing, defensive block compression and formation recovery.
- AI decision-making for short passes, through balls, wide crosses and shots based on receiver runs, passing-lane clearance and field zone.
- AI kicks use the existing physical ball with target direction, foot-contact range checks and action telemetry.
- Test builder assigns 4-3-3 squad slots to the human and AI players.

See [Football Tactical AI Engine v0.7](docs/FOOTBALL_TACTICAL_AI_V0_7.md).

### Football Match Engine v0.8
- Match Director with configurable halves, accelerated test clock, half-time side switch and full-time state.
- Team-based score tracking from goal events.
- Logical referee decisions for kick-offs, throw-ins, corners, goal kicks, direct / indirect free kicks and penalties.
- Basic offside-position snapshot when a teammate deliberately plays the ball; offence is called when an identified player touches it.
- Ball last-touch tracking and physical placement for restarts.
- Goalkeeper AI positioning, close-range parry and goal-kick distribution.
- Test builder spawns 22 players: one human-controlled player and 21 AI.

See [Football Match Engine v0.8](docs/FOOTBALL_MATCH_ENGINE_V0_8.md).

**Important:** these are development blockouts and starter systems, not a finished photoreal stadium or finished football gameplay. Unreal C++ compilation and visual verification still need to happen on the target Windows machine.

## Run the Python backend (Windows PowerShell)

Requires Python 3.11 or newer.

~~~powershell
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r backend\requirements.txt
python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
~~~

Or run the setup/start script:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\run_backend.ps1
~~~

Open:
- http://127.0.0.1:8000/health
- http://127.0.0.1:8000/docs
- http://127.0.0.1:8000/api/v1/clubs
- http://127.0.0.1:8000/api/v1/players

Run backend tests from the repository root:

~~~powershell
$env:PYTHONPATH = "backend"
python -m pytest backend/tests -q
python tools/validate_data.py
~~~

For setup in Vietnamese, see [docs/GETTING_STARTED_VI.md](docs/GETTING_STARTED_VI.md).

## Data integrity and rights

The files under data/fixtures are synthetic development fixtures, not real or official football data. Production data must have documented source, license/permission, attribution, retrieval date and verification status. Do not scrape or redistribute protected databases, club crests, kits, photographs, player likenesses or proprietary game assets without appropriate rights.

## Development workflow

- Work on develop or a feature branch; propose integration into main through a pull request.
- Run the Python tests before proposing backend changes.
- Large Unreal binary assets should use Git LFS; generated build/cache folders are ignored.
- Never commit credentials, private tokens, local databases or user data.

## Current status

The repository contains the Python API foundation, stadium/camera C++ starter, player movement/animation starter, physics ball, assisted dribbling, passing, through balls, shooting, goal detection, team-aware AI movement / support / press / marking, Editor Python builders and a PowerShell installer. The Unreal project must still be generated, compiled and play-tested in the local Unreal Editor + MSVC environment. Actual animation clips and montage assets are not bundled: the v0.4 C++ timing hooks require the user to import licensed animations, add the two foot sockets and place contact notifies. The v0.8 prototype includes a 22-player test lineup, Match Director clock / team score, out-of-play restarts, a basic offside-position snapshot, goalkeeper positioning / parry / distribution and a Blueprint foul-award hook. The referee does not yet detect every foul automatically; the goalkeeper parries rather than running a complete catch / dive animation, and the offside model does not yet evaluate every form of interfering with opponents or gaining an advantage from rebounds. Complete match rules, 11v11 AI, official worldwide roster and online mode are not claimed yet.
