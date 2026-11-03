# Football Tactical AI Engine v0.7 — Formations, Passing and Phase Transitions

This milestone builds on v0.6. It introduces formation-driven position targets, attacking/defending phases and target-aware AI ball actions. It is a tactical prototype rather than a completed 11v11 football simulation.

## Formation presets

Every footballer exposes:
- TeamFormation: Formation433, Formation4231 or Formation442
- SquadSlotIndex: 0 to 10 (0 is goalkeeper; 1–4 are the back four; 5–10 are midfielders / attackers)
- TeamRole: optional explicit role override
- AttackDirectionSign: +1 toward positive X, -1 toward negative X

The same TeamFormation should be assigned to every member of one team. The test builder uses 4-3-3 by default.

### Slot map

| Slot | 4-3-3 | 4-2-3-1 | 4-4-2 |
|---:|---|---|---|
| 0 | GK | GK | GK |
| 1–4 | RB, RCB, LCB, LB | RB, RCB, LCB, LB | RB, RCB, LCB, LB |
| 5–7 | DM, RCM, LCM | RDM, LDM, CAM | RM, RCM, LCM |
| 8 | RW | RAM | LM |
| 9 | ST | ST | ST |
| 10 | LW | LAM | ST |

Formation offsets are relative to the ball and rotate with AttackDirectionSign. During defending, the team is compressed toward its own goal and the lateral lanes are narrowed. Goalkeeper slot 0 stays near its home location and shifts laterally with the ball.

## Match phases and pressing lines

The AI publishes:
- TransitionToAttack when the ball is loose or the team wins possession
- Attacking after the short attacking transition window
- TransitionToDefence when possession turns over to the opponent
- Defending after the short defensive transition window

In possession, the ball carrier advances if no viable pass or shot is available. Teammates run to their formation lanes. Out of possession:
- The closest eligible forward / winger / midfielder leads the press.
- Defenders step out only after the ball enters the team's defensive block.
- Other players mark threatening opponents or recover toward compact formation targets.
- Close contest invokes the v0.5 balance / brace response with a cooldown.

## AI ball decisions

The player controller calls the target-based TryExecuteAITacticalKick API. The ball is still a physics actor and the kick is rejected unless the ball is close enough to the selected foot contact point.

| Action | Selection |
|---|---|
| Short pass | Safe receiver with a clear passing lane and useful support spacing |
| Through ball | Receiver making a forward run with additional lane clearance |
| Cross | Wide attacking channel, late field zone, and a central runner / far-post target |
| Shot | Ball carrier in range of the goal and within the central shooting corridor |
| Carry | No sufficiently safe pass/shot target; keep advancing with the ball |

Pass evaluation uses receiver position and velocity, forward progress, distance and the nearest-opponent clearance from the pass segment. Crosses apply a larger upward impulse to create a lofted trajectory. AI action and target state are exposed to Animation Blueprint.

## Install and test on Windows

From the repository root:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

Close Unreal Editor before installation, regenerate Visual Studio project files and build the Development Editor target. Reopen Unreal Editor and run:

~~~text
Content/Python/build_player_test_level.py
~~~

Press Play. The builder creates one human-controlled player and seven AI footballers split between two teams. The current test uses assigned 4-3-3 slot indices.

## Tuning

On a Blueprint child of FootballTeamAIController, adjust:
- DecisionIntervalSeconds and PressingRadiusCm
- ShotRangeCm and MaximumPassRangeCm
- MinimumPassLaneClearanceCm and CrossZoneDepthCm
- ContestDistanceCm and bEnableAutomaticContestImpacts

On each footballer, set TeamFormation, SquadSlotIndex, TeamRole and AttackDirectionSign. Team members on the same side should share the same TeamFormation and AttackDirectionSign.

## Current limitations

- Possession is estimated by nearest-player distance, not a server-authoritative ball-owner component.
- The current attacking / defending transition model is phase-based but does not yet model every trigger such as turnovers, counter-press windows, rest defence or time/score context.
- The AI selects and executes kicks, but it does not yet run AI-specific pass/cross/shot Montages. Human-controlled kick animations still use the v0.4 contact-notify pipeline.
- A cross is a lofted directional kick toward a central runner, not a fully authored cross-animation / ball-flight model.
- Formation presets position active players, but the test scene is not 11v11. No goalkeeper dive/catch AI, advanced offside, or tactical foul logic is implemented.
- Steering still does not perform NavMesh pathfinding or robust obstacle avoidance. A crowded route can remain awkward.
- C++ must still be compiled and play-tested in the local Unreal Engine + MSVC environment. CI checks Python tests, source symbols and builder syntax only; it does not compile Unreal C++ or validate in-game tactics.
