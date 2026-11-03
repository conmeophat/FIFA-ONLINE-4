# Football Match Simulation v0.9 — Player Duels, Goalkeeper Possession, Set Pieces and HUD

v0.9 deepens the offline 11v11 match slice on top of the v0.8 Match Director. It adds player-to-player contact handling, a goalkeeper catch/hold/distribution path, restart-specific AI choices, and a native UMG scoreboard.

## 1. Player contact and shoulder duels

Each footballer periodically checks nearby opponents. A pair is processed by one deterministic initiator (the actor with the lower unique ID), avoiding duplicate impulses from both characters. The contact resolver considers separation, relative speed, bracing and whether either player is controlling the ball.

When a duel is registered, both players receive a configurable velocity impulse and balance damage through the existing `ApplyContestImpact` path. The Animation Instance exposes:
- `bIsInShoulderDuel`
- `ContestReactionStrength`
- the existing balance, off-balance, bracing and contest-stun state

The variables are intended to drive a blend between locomotion and authored shoulder / stumble / recovery poses. Licensed animation assets are not included in this repository.

## 2. Goalkeeper catch, hold and distribution

The goalkeeper first attempts to catch a reachable, manageable ball in its own penalty area. A successful catch stops rigid-body simulation, disables ball collision and attaches the ball to the keeper's root component. The keeper then evaluates teammates and releases a targeted lofted distribution. Stronger or unreachable shots continue to use the v0.8 parry response.

This is a functional possession state, not a finished hand-grip system: a production rig should replace the root-relative ball offset with a hand socket and add catch, dive, get-up and distribution Montages.

## 3. Set-piece AI

The restart taker still approaches the stopped ball and uses the physical ball actor. The selected target now depends on the restart:
- Kick-off: a short, central first pass.
- Corner: a lofted delivery toward the central six-yard / penalty-spot channel; attacking support players make box runs.
- Direct / indirect free kick: direct shot from the central shooting zone, otherwise a delivery into the box.
- Throw-in: a shorter pass to a nearby outlet.
- Goal kick: a wide outlet or long distribution.

These are deterministic prototype heuristics, not a complete playbook editor. Human restart kicks still use the existing keyboard action and physical strike profile. A dedicated throw-in animation / overhand throw is not bundled.

## 4. Native match HUD

The Match Director creates `UFootballMatchHUDWidget` and adds it to the local player viewport. The default widget displays:
- Team 0 and Team 1 score
- match clock
- current period (first half, half-time, second half or full-time)

The widget is deliberately small and native. A project can subclass it through `MatchHUDWidgetClass` to apply a custom broadcast scoreboard design. The installer now adds the `UMG`, `Slate` and `SlateCore` module dependencies.

## 5. Install on Windows

From the repository root:

~~~powershell
git switch main
git pull origin main
Set-ExecutionPolicy -Scope Process Bypass
.\tools\install_unreal_source.ps1
~~~

Then close Unreal Editor, regenerate Visual Studio project files, build the Development Editor target, reopen Unreal and run:

~~~python
py "Content/Python/build_player_test_level.py"
~~~

Press Play. The builder spawns one human player and 21 AI players, the physical ball and Match Director. The HUD is created automatically when the local Player Controller is available.

## 6. Validation boundary and limitations

The repository's Python CI can validate source-bundle completeness, expected C++ symbols, installer wiring and Editor Python syntax. It does not compile the Unreal C++ module with MSVC or launch a 3D match.

The following still require the local Unreal project and a visual playtest:
- Compile the new UMG class and all runtime source with the installed UE version / MSVC toolchain.
- Verify the HUD appears in the project Game viewport and adjust its layout for the target resolution.
- Tune duel radii / impulses to prevent excessive knockback or AI pile-ups.
- Configure hand sockets and import licensed keeper / shoulder-contact / locomotion animations.
- Verify goalkeeper catch and release while the ball is moving and at goal-area boundaries.

Foul detection is still provided through the v0.8 Blueprint-callable referee hook; this milestone does not claim automatic foul recognition. Offside remains a simplified position snapshot plus active-ball-touch check. No official worldwide player database, club assets, finished FC Online-style animations, online mode or complete law-of-the-game implementation is claimed.
