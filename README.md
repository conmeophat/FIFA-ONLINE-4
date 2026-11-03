# FIFA ONLINE 4 — Python + Unreal Football Project

> An independent football game project inspired by online football management and match simulations. It is not affiliated with EA or the original FIFA / FC Online products.

## Technology direction

- **Game client:** Unreal Engine 5; C++ and Blueprints handle packaged-game runtime gameplay.
- **Python:** FastAPI backend, football-data validation/import, management rules, editor automation and developer tools.
- **First target:** a playable offline prototype with a broadcast-style 3D camera.
- **Later target:** online sessions after the offline match simulation is stable.

### Important Unreal/Python boundary

Unreal's Python integration is an **Editor scripting** environment, not the runtime gameplay language for packaged games. Player input, movement, animation, collision, ball physics and match simulation belong in Unreal C++/Blueprints. Python provides data, APIs and tooling. See [the architecture](docs/ARCHITECTURE.md).

## Repository layout

```text
.
├── backend/          # FastAPI service, domain models and tests
├── data/fixtures/    # Synthetic development data only
├── docs/             # Architecture, roadmap, data policy and Windows guide
├── tools/            # Local run and data validation scripts
├── unreal/           # Unreal setup guide and C++ integration starter
└── .github/          # GitHub Actions CI
```

## Run the backend (Windows PowerShell)

Requires Python 3.11 or newer.

```powershell
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r backend\requirements.txt
python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
```

Or run the setup/start script:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\run_backend.ps1
```

Open:
- http://127.0.0.1:8000/health
- http://127.0.0.1:8000/docs
- http://127.0.0.1:8000/api/v1/clubs
- http://127.0.0.1:8000/api/v1/players

Run tests from the repository root:

```powershell
$env:PYTHONPATH = "backend"
python -m pytest backend/tests -q
python tools/validate_data.py
```

For a complete Vietnamese setup walkthrough, see [docs/GETTING_STARTED_VI.md](docs/GETTING_STARTED_VI.md).

## Data integrity and rights

The files under `data/fixtures/` are **synthetic development fixtures**, not real or official football data. Production data must have documented source, license/permission, attribution, retrieval date and verification status. Do not scrape or redistribute protected databases, club crests, kits, photographs, player likenesses or proprietary game assets without appropriate rights.

## Development workflow

- Work on `develop` or a feature branch; propose integration into `main` through a pull request.
- Run the Python tests before proposing backend changes.
- Large Unreal binary assets should use Git LFS; generated build/cache folders are ignored.
- Never commit credentials, private tokens, local databases or user data.

## Current status

Repository foundation and Python API skeleton are present. The Unreal project itself must be generated with the installed Unreal Editor on the development PC; a playable match, official worldwide roster and online mode do not exist yet.
