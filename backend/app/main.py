"""HTTP API for the offline-first football game prototype."""

from fastapi import FastAPI, HTTPException, Query

from .repository import load_clubs, load_leagues, load_players

app = FastAPI(
    title="Python Football Online API",
    description=(
        "Offline-first football game backend. Initial records are synthetic "
        "development fixtures, not an official player or club database."
    ),
    version="0.1.0",
)


@app.get("/", tags=["system"])
def home() -> dict[str, str]:
    return {
        "project": "Python Football Online",
        "version": app.version,
        "status": "running",
        "mode": "offline-first",
    }


@app.get("/health", tags=["system"])
def health() -> dict[str, str]:
    return {"status": "healthy", "service": "football-backend"}


@app.get("/api/v1/meta", tags=["system"])
def dataset_metadata() -> dict[str, object]:
    clubs = load_clubs()
    leagues = load_leagues()
    players = load_players()
    return {
        "dataset_type": "synthetic-development-fixtures",
        "official_data_loaded": False,
        "counts": {
            "clubs": len(clubs),
            "leagues": len(leagues),
            "players": len(players),
        },
    }


@app.get("/api/v1/clubs", tags=["clubs"])
def list_clubs(q: str | None = Query(default=None, min_length=1)) -> dict[str, object]:
    clubs = load_clubs()
    if q:
        term = q.casefold()
        clubs = [club for club in clubs if term in club.name.casefold()]
    return {"count": len(clubs), "items": [club.model_dump(mode="json") for club in clubs]}


@app.get("/api/v1/clubs/{club_id}", tags=["clubs"])
def get_club(club_id: str) -> dict[str, object]:
    for club in load_clubs():
        if club.id == club_id:
            return club.model_dump(mode="json")
    raise HTTPException(status_code=404, detail="Club not found")


@app.get("/api/v1/leagues", tags=["leagues"])
def list_leagues() -> dict[str, object]:
    leagues = load_leagues()
    return {"count": len(leagues), "items": [league.model_dump(mode="json") for league in leagues]}


@app.get("/api/v1/players", tags=["players"])
def list_players(
    club_id: str | None = None,
    position: str | None = Query(default=None, min_length=1),
    q: str | None = Query(default=None, min_length=1),
    min_pace: int = Query(default=0, ge=0, le=100),
) -> dict[str, object]:
    players = load_players()

    if club_id:
        players = [player for player in players if player.current_club_id == club_id]

    if position:
        normalized_position = position.strip().upper()
        players = [
            player for player in players
            if normalized_position in [item.upper() for item in player.positions]
        ]

    if q:
        term = q.casefold()
        players = [player for player in players if term in player.full_name.casefold()]

    players = [player for player in players if player.attributes.pace >= min_pace]
    return {"count": len(players), "items": [player.model_dump(mode="json") for player in players]}


@app.get("/api/v1/players/{player_id}", tags=["players"])
def get_player(player_id: str) -> dict[str, object]:
    for player in load_players():
        if player.id == player_id:
            return player.model_dump(mode="json")
    raise HTTPException(status_code=404, detail="Player not found")
