"""Validated domain models for football reference data."""

from typing import Literal

from pydantic import BaseModel, ConfigDict, Field


class StrictModel(BaseModel):
    model_config = ConfigDict(extra="forbid")


class DataSource(StrictModel):
    provider: str = Field(min_length=1)
    source_url: str | None = None
    license: str = Field(min_length=1)
    retrieved_at: str | None = None
    verification_status: Literal["verified", "unverified", "synthetic"]


class PlayerAttributes(StrictModel):
    pace: int = Field(ge=0, le=100)
    shooting: int = Field(ge=0, le=100)
    passing: int = Field(ge=0, le=100)
    dribbling: int = Field(ge=0, le=100)
    defending: int = Field(ge=0, le=100)
    physical: int = Field(ge=0, le=100)


class Club(StrictModel):
    id: str = Field(min_length=1)
    name: str = Field(min_length=1)
    country_code: str = Field(min_length=2, max_length=3)
    competition_ids: list[str] = Field(default_factory=list)
    is_fictional: bool
    source: DataSource


class League(StrictModel):
    id: str = Field(min_length=1)
    name: str = Field(min_length=1)
    country_code: str = Field(min_length=2, max_length=3)
    season_label: str = Field(min_length=1)
    club_ids: list[str] = Field(default_factory=list)
    is_fictional: bool
    source: DataSource


class Player(StrictModel):
    id: str = Field(min_length=1)
    full_name: str = Field(min_length=1)
    current_club_id: str | None = None
    positions: list[str] = Field(min_length=1)
    attributes: PlayerAttributes
    is_fictional: bool
    source: DataSource
