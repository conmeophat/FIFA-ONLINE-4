"""Read and validate the local football dataset.

The initial repository uses small JSON fixtures. A licensed production data
provider can be integrated behind these functions without changing the API.
"""

import json
import os
from pathlib import Path
from typing import TypeVar

from pydantic import ValidationError

from .models import Club, League, Player, StrictModel

ModelT = TypeVar("ModelT", bound=StrictModel)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_DATA_DIR = REPOSITORY_ROOT / "data" / "fixtures"
DATA_DIR = Path(os.getenv("FOOTBALL_DATA_DIR", str(DEFAULT_DATA_DIR)))


class DatasetLoadError(RuntimeError):
    """Raised when the configured dataset is missing or invalid."""


def _load_collection(filename: str, collection_key: str, model: type[ModelT]) -> list[ModelT]:
    path = DATA_DIR / filename

    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise DatasetLoadError(f"Dataset file is missing: {filename}") from exc
    except json.JSONDecodeError as exc:
        raise DatasetLoadError(f"Dataset file is not valid JSON: {filename}") from exc

    if not isinstance(payload, dict) or not isinstance(payload.get(collection_key), list):
        raise DatasetLoadError(
            f"Dataset {filename} must contain a list named '{collection_key}'."
        )

    records: list[ModelT] = []
    for index, item in enumerate(payload[collection_key]):
        try:
            records.append(model.model_validate(item))
        except ValidationError as exc:
            raise DatasetLoadError(
                f"Invalid record {index} in {filename}: {exc}"
            ) from exc

    return records


def load_clubs() -> list[Club]:
    return _load_collection("clubs.json", "clubs", Club)


def load_leagues() -> list[League]:
    return _load_collection("leagues.json", "leagues", League)


def load_players() -> list[Player]:
    return _load_collection("players.json", "players", Player)
