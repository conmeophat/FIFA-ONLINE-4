# Football Team AI Engine v0.6 — Support Runs, Space, Pressing and Marking

This milestone adds the first playable team-AI layer to the offline Unreal prototype. It is a lightweight steering-based football AI, not a full tactical simulation or 11v11 match engine.

## What is implemented

### 1. Team identity and roles
- Every AFootballPlayerCharacter exposes TeamId, AttackDirectionSign and TeamRole.
- TeamId separates teammates from opponents.
- AttackDirectionSign is +1 when the team attacks toward positive X and -1 when it attacks toward negative X.
- Roles are Goalkeeper, Defender, Midfielder, Winger and Forward. The role changes the preferred support position / forward offset.

### 2. AI possession and loose-ball decisions
- AFootballAIPlayerCharacter automatically creates an AFootballTeamAIController when placed in the world.
- The controller periodically scans footballers and the ball to estimate the nearest ball carrier.
- When the ball is loose, only the nearest footballer chases it; other AI players recover toward their home shape.
- When an AI footballer has the ball, it carries forward using the existing v0.5 movement and v0.3 ball-control systems.

### 3. Off-ball movement and finding space
- Teammates without the ball run into support positions rather than following the ball carrier.
- The controller evaluates a set of candidate lanes around a role-based anchor.
- Candidate scoring rewards distance from opponents and teammates, favours forward progress, and penalizes excessive distance from the tactical anchor / home position.
- A separation steering term reduces AI teammate stacking at close range.

### 4. Coordinated defending
- The nearest teammate to the ball carrier is assigned to press.
- Other defenders select a threatening opponent, with a penalty when a teammate is already closer to that opponent.
- Marking positions are shifted toward the defending team's own goal to protect the goal-side lane.
- The designated presser invokes ApplyContestImpact at close range. This is a low-strength shoulder-contact prototype with a cooldown and uses the v0.5 balance / brace response.

### 5. Animation Blueprint integration
The FootballPlayerAnimInstance exposes:
- bIsAIControlled
- AITaskState: Idle, ChaseBall, CarryBall, SupportRun, FindSpace, PressOpponent, MarkOpponent or RecoverShape.

Use these values to blend task-specific locomotion or defensive stance animations in an Animation Blueprint.

## Test on Windows

Run from the repository root:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

Close Unreal Editor before installing source. Regenerate project files, build the Development Editor target in Visual Studio, then reopen the project. In the Unreal Editor, run:

~~~text
Content/Python/build_player_test_level.py
~~~

Press Play. The builder creates one human-controlled Team 0 footballer, seven AI footballers split across Team 0 and Team 1, and one physics ball.

Controls remain:
- WASD: move
- Mouse: camera
- Left Shift: sprint
- E: pass
- Q: through ball
- Left Mouse Button: shoot
- Left Control: brace

## Tuning in Unreal

Tune DecisionIntervalSeconds, StopDistanceCm, SlowDownDistanceCm, BallPossessionRadiusCm, PressingRadiusCm, ContestDistanceCm, FieldHalfLengthCm, FieldHalfWidthCm and bEnableAutomaticContestImpacts on a Blueprint child of FootballTeamAIController.

Set TeamRole on the character to change its default support lane. Keep both teams' AttackDirectionSign values opposite.

## Current limitations

- This is a local steering prototype. It does not use NavMesh pathfinding and does not avoid every obstacle; players can still collide or choose awkward routes in crowded spaces.
- Possession is estimated from nearest-player distance, not a full ball-owner / tackle / interception authority system.
- AI support players move into space but do not yet evaluate passing options, automatically request a pass, or execute autonomous pass / shot decisions.
- Goalkeepers do not yet have diving, catching, distribution or penalty-area logic.
- The prototype test is a small scrimmage (one human plus three AI against four AI), not full 11v11.
- Character C++ still needs compilation and gameplay tuning in the local Unreal Engine + MSVC installation. Repository CI validates source symbols and Python syntax; it does not compile Unreal C++ or prove in-engine movement feel.
