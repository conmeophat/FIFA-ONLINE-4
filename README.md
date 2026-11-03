# FIFA ONLINE 4 — Python + Unreal Football Project

> A community-built football game project inspired by online football management and match simulations. This is an independent project and is not affiliated with EA or the original FIFA/FC Online products.

## Chosen direction

- **Game client:** Unreal Engine 5 (C++ and Blueprints for runtime gameplay).
- **Python:** backend services, data ingestion/validation, editor automation, AI research/prototypes, and developer tools.
- **Initial milestone:** offline vertical slice; multiplayer is intentionally postponed.
- **Visual target:** realistic 3D football presentation with a broadcast-style camera.

### Important engine boundary

Unreal's Python integration is an **Editor scripting** environment, not the runtime gameplay language for packaged games. The match simulation, player controller, ball physics, animation state and client networking must run in Unreal C++/Blueprints. Python will provide the data/API/tooling layer. This boundary avoids designing a runtime architecture around a capability Unreal does not provide.

## Repository layout

```text
.
├── backend/          # Python FastAPI service and domain models
├── data/             # Small, clearly-labelled development fixtures
├── docs/             # Architecture, roadmap and data policy
├── tools/            # Data validation and developer utilities
├── unreal/           # Unreal project integration notes / future project
└── .github/          # CI workflows
```

## First milestone

1. Run the Python backend locally.
2. Validate sample clubs and players through HTTP endpoints.
3. Create the Unreal C++ project in the Epic Games Launcher and place it under `unreal/`.
4. Connect the Unreal client to the local backend.
5. Build an offline match prototype before designing online authority and matchmaking.

## Run the backend (Windows PowerShell)

Requires Python 3.11+.

```powershell
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r backend\requirements.txt
python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
```

Open:
- http://127.0.0.1:8000/health
- http://127.0.0.1:8000/docs
- http://127.0.0.1:8000/api/v1/clubs
- http://127.0.0.1:8000/api/v1/players

## Data integrity and rights

The files under `data/` are **development fixtures only**, not a complete or official football database. Do not infer that ratings, player attributes, club affiliations, competition membership, crests, kits, likenesses or stadium assets are official. Every production data source must be recorded with its license/permission, attribution, retrieval date and verification status. Do not scrape or redistribute protected databases, logos, photographs, player likenesses or proprietary game assets without appropriate rights.

## Development workflow

- Keep changes on a feature branch and open a pull request into `main`.
- Run `python -m pytest` before proposing backend changes.
- Keep Unreal binary assets out of Git unless using Git LFS; generated build/cache folders must be ignored.
- Never commit credentials, private tokens, local databases, or user data.

## Status

Bootstrap stage — API skeleton and documented architecture are being added. Unreal project assets and a playable match have not yet been created.
