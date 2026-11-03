# Football Match Engine v0.8 — 11v11, Clock, Referee Rules and Goalkeepers

v0.8 adds a central Match Director to the Unreal prototype. It coordinates match time, team score, referee decisions and restarts while the player, ball and tactical AI systems handle physical movement.

## Match format

- Two teams use IDs 0 and 1.
- The test builder spawns 22 footballers: one human-controlled player and 21 AI players.
- Both teams start with a 4-3-3 slot layout: 0 goalkeeper, 1–4 defenders, 5–7 midfielders and 8–10 attackers.
- Team 0 starts attacking +X; Team 1 attacks -X. Directions switch at half-time.
- The Match Director owns period, clock, score, current restart and referee rule state.

## Match clock and score

- First- and second-half lengths are configured by HalfDurationSeconds (default 2700 game seconds).
- MatchClockRate=1 is real-time. The test builder uses 30x for rapid testing.
- A configurable half-time break precedes second-half kick-off.
- The clock stops at half-time, during goal restart setup and at full-time.
- Team score is tracked by team ID, independently of the ball actor's physical +X / -X goal counters.
- GetMatchClockText exposes a scoreboard-friendly MM:SS value; OnScoreChanged and OnRestartAwarded are Blueprint events.

## Referee logic and restarts

The Match Director checks the ball after physics and classifies:
- Touchline exit: throw-in to the team that did not touch the ball last.
- End-line exit after an attacking-team touch: goal kick to the defending team.
- End-line exit after a defending-team touch: corner to the attacking team.
- Goal event: kick-off to the team that conceded.
- Offside touch: indirect free kick to the opponents.
- SignalFoul is a Blueprint-callable referee hook. A tackle / collision system can report a foul and indicate whether it occurred inside the penalty area; the director awards a direct free kick or penalty.

The ball stores its latest touch team and actor. For a restart, the director physically stops the ball at the awarded location. The team awarded the decision must take it; the clock resumes after a valid restart kick.

## Basic offside model

When a player deliberately plays a short pass, through ball, cross, free kick, penalty or shot, the Match Director snapshots attacking teammates who are:
- in the opponent's half;
- nearer the opponent goal than the ball; and
- beyond the second-last opponent when the ball is played.

An identified player is penalized when they subsequently touch the ball. The model deliberately does not evaluate every law nuance, such as interfering with an opponent, blocking the goalkeeper's view or gaining an advantage from a rebound. Throw-ins, corners and goal kicks do not create an offside offence directly.

## Goalkeeper AI

A goalkeeper is recognized from SquadSlotIndex=0 unless an explicit TeamRole override is set. The AI:
- stays near its own goal and shifts laterally with the ball;
- advances into its own penalty area for a nearby loose ball;
- attempts a physical parry when a shot is moving toward its goal and enters keeper reach;
- distributes with a lofted goal-kick action after close control.

The current save is a physics deflection, not a complete hand-catching state, authored dive or goalkeeper possession system.

## Install and run on Windows

From the repository root:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

Close Unreal Editor, regenerate Visual Studio project files and build the Development Editor target. Reopen Unreal Editor and run:

~~~text
Content/Python/build_player_test_level.py
~~~

Press Play. The builder creates 22 players, one physical ball and one Match Director. The test clock is 30x by default; set MatchClockRate to 1 for real-time pacing.

## Limitations

- The 11v11 setup is a test lineup using primitive proxy players; licensed 3D models, rigs and animation clips are not bundled.
- Offside detects position and active ball touch, not every form of interfering with play or gaining an advantage.
- SignalFoul is an API hook; fouls are not yet detected automatically for every tackle / collision.
- The goalkeeper parries and distributes. There is no complete hand catch, dive montage, penalty anticipation or rebound ownership logic.
- Restart placement is functional prototype logic; authored throw-in / corner / free-kick animations, player walls and set-piece run patterns are not yet included.
- The Match Director assumes exactly two sides with TeamId 0 and 1.
- AI movement still lacks robust NavMesh obstacle avoidance. C++ must be compiled and play-tested in the target Unreal Engine + MSVC environment. GitHub CI validates Python tests and source symbols only; it does not compile Unreal C++ or validate runtime match behavior.
