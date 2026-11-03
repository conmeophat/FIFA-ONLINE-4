# Development roadmap

## Milestone 0 — Repository foundation
- [x] Python API skeleton and validated domain models
- [x] Synthetic fixture data labelled as fictional
- [x] API tests and CI
- [x] Windows setup guide and local backend runner
- [ ] Local developer environment verified on the target PC
- [ ] Unreal C++ project generated and checked in (with Git LFS for binary assets)

## Milestone 1 — Stadium and broadcast camera blockout
- [x] Scale-aware pitch surface and football markings (C++ construction actor)
- [x] Penalty areas, goal frames and corner flags
- [x] Three-tier stand blockout and floodlight masts
- [x] Smooth high sideline broadcast camera with tracked actor and pitch clamps
- [x] Unreal Editor Python script to place the actors in the current level
- [x] PowerShell installer to copy C++ sources and add module dependencies
- [ ] Compile the C++ project on the target Windows / Unreal installation
- [ ] Verify the level visually and assign project materials / lighting

## Milestone 2 — Single-player football prototype
- [ ] Possess one controllable footballer
- [ ] Add basic locomotion and animation state
- [ ] Add ball collision, dribble touch, pass and shot prototypes
- [ ] Add goal detection, match clock and score
- [ ] Keep match simulation entirely in Unreal C++/Blueprint

## Milestone 3 — Offline match slice
- [ ] Add goalkeeper and simple teammate/opponent AI
- [ ] Build restart states (kick-off, goal, throw-in, corner, free kick)
- [ ] Add substitution and match summary
- [ ] Profile CPU/GPU and validate stable frame pacing

## Milestone 4 — Football management
- [ ] Connect Unreal to the Python catalogue API
- [ ] Add squads, formations, player positions and team tactics
- [ ] Add local saves and career progression
- [ ] Introduce a licensed, documented real-world data provider only after legal and schema review

## Milestone 5 — Online foundation
- [ ] Design account/session services and an authoritative match server
- [ ] Implement matchmaking and reconnect handling
- [ ] Add server-side validation, rate limits, logging and anti-cheat controls
- [ ] Load test before public deployment

Each milestone should be independently runnable and tested. Do not claim 11v11, official worldwide rosters, online play or FC Online-level fidelity until those features exist and have been verified.
