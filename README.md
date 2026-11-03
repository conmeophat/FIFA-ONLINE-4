# FIFA ONLINE 4 — Python + Unreal Football Project

> An independent football game project inspired by online football management and match simulations. It is not affiliated with EA or the original FIFA / FC Online products.

## Technology direction

- **Game client:** Unreal Engine 5; C++ and Blueprints handle packaged-game runtime gameplay.
- **Python:** FastAPI backend, football-data validation/import, management rules, editor automation and developer tools.
- **First target:** a playable offline prototype with a broadcast-style 3D camera.
- **Later target:** online sessions after the offline match simulation is stable.

Unreal's Python integration is an Editor scripting environment, not the runtime gameplay language for packaged games. Player input, movement, animation, collision, ball physics and match simulation belong in Unreal C++/Blueprints. Python provides data, APIs and tooling. See the [technical architecture](docs/ARCHITECTURE.md).

## Repository layout

~~~text
.
├── backend/          # FastAPI service, domain models and tests
├── data/fixtures/    # Synthetic development data only
├── docs/             # Architecture, roadmap, data policy and Windows guides
├── tools/            # Local run, validation and Unreal source installer
├── unreal/           # C++ actors, Editor Python builder and setup guide
└── .github/          # GitHub Actions CI
~~~

## Current Unreal stadium milestone

The repository now contains:
- A C++ stadium actor that builds a scale-aware pitch, lines, penalty boxes, goal frames, corner flags, stand blockout and floodlight masts.
- A C++ broadcast camera actor that tracks a target with smooth pan and pitch-boundary clamps.
- An Unreal Editor Python script that places the actors into the currently open level.
- A PowerShell installer that copies starter C++ sources into the generated Unreal project and adds missing HTTP/JSON module dependencies.

This is a geometry blockout, not a finished photoreal stadium. The local Unreal project still needs to be generated, compiled and visually checked on the development PC.

See [Stadium 3D + Broadcast Camera guide](docs/STADIUM_BROADCAST_CAMERA.md).

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

The Python API foundation and stadium/camera C++ starter sources are in the repository. The Unreal project itself must still be generated and compiled in the local Unreal Editor + MSVC environment. A playable football match, official worldwide roster and online mode do not exist yet.
