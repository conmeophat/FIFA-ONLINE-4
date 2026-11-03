# Technical architecture

## Product constraints

The project targets a realistic 3D football game with a broadcast camera. Development is offline-first. Multiplayer is a later milestone and must not compromise the first playable offline slice.

## Runtime responsibility split

| Layer | Technology | Responsibilities |
|---|---|---|
| Game client | Unreal Engine 5 C++ / Blueprint | Player input, character movement, animation, ball physics, collision, camera, match rules, rendering and audio |
| Editor automation | Unreal Python Editor API | Asset import, batch renaming, level setup, validation and editor workflows |
| Domain/backend | Python + FastAPI | Club/player/league data, squad rules, roster validation, career and transfer business logic |
| Persistence | JSON fixtures initially; PostgreSQL later | Durable reference and career data |
| Online layer (future) | Authoritative server + WebSocket/UDP design | Matchmaking, session authority, replication, anti-cheat and results |

**Do not put packaged-game real-time gameplay in Unreal's Python Editor plugin.** Unreal Python is for editor automation. C++/Blueprints own runtime gameplay.

## Offline request flow

1. Unreal starts an offline session.
2. The client requests catalogue and squad data from the local Python service over HTTP.
3. The backend validates and returns versioned JSON.
4. Unreal converts the response into native gameplay structures.
5. Match simulation continues locally in Unreal; a temporary backend outage must not interrupt an already-started offline match.

## Match simulation boundaries

The Unreal client owns:
- 3D scene, pitch, stadium, lights, broadcast camera and UI.
- Local player input and controller mapping.
- Player movement, animation state, collision and ball physics.
- Match clock, score, restarts, fouls and offline opponent AI.

The Python service owns:
- Canonical catalogue IDs and data provenance.
- Squad composition and formation validation.
- Club, league, season and transfer-market business rules.
- Save metadata and future account services.

Do not run a 60 Hz match simulation through HTTP. API calls are for setup, persistence and business operations, not per-frame physics.

## API conventions

- Prefix: `/api/v1`.
- JSON responses use stable IDs rather than display names as foreign keys.
- Invalid IDs return HTTP 404; invalid query values are rejected by FastAPI.
- Every production data record must carry source, license, retrieval time and verification status.
- Development fixture data is explicitly synthetic.

## Broadcast camera

The first camera prototype should use a spring-arm / camera rig controlled by match phase and ball position. Use a high sideline broadcast perspective, smooth follow interpolation, pitch-boundary clamps, and a separate close-up/replay camera later. Camera tuning belongs in Unreal, not Python.

## Future online architecture

For multiplayer, move match authority to a dedicated authoritative simulation/server. Clients send validated inputs; the server owns ball/player state and match outcomes. Do not trust a client-reported score or transfer balance. Select replication and transport after latency and determinism tests; do not treat the initial REST API as a real-time netcode solution.
