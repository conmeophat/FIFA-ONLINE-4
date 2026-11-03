from fastapi.testclient import TestClient

from app.main import app

client = TestClient(app)


def test_health_endpoint() -> None:
    response = client.get("/health")
    assert response.status_code == 200
    assert response.json()["status"] == "healthy"


def test_metadata_explicitly_marks_fixture_dataset() -> None:
    response = client.get("/api/v1/meta")
    assert response.status_code == 200
    payload = response.json()
    assert payload["dataset_type"] == "synthetic-development-fixtures"
    assert payload["official_data_loaded"] is False


def test_club_list_and_search() -> None:
    all_clubs = client.get("/api/v1/clubs")
    assert all_clubs.status_code == 200
    assert all_clubs.json()["count"] >= 1

    filtered = client.get("/api/v1/clubs", params={"q": "blue"})
    assert filtered.status_code == 200
    assert filtered.json()["count"] == 1


def test_player_filters_are_combined() -> None:
    response = client.get(
        "/api/v1/players",
        params={"club_id": "demo_blue_fc", "position": "st", "min_pace": 70},
    )
    assert response.status_code == 200
    assert response.json()["count"] == 1


def test_fixture_players_are_not_presented_as_real_players() -> None:
    response = client.get("/api/v1/players")
    assert response.status_code == 200
    for player in response.json()["items"]:
        assert player["is_fictional"] is True
        assert player["source"]["verification_status"] == "synthetic"


def test_unknown_player_returns_404() -> None:
    response = client.get("/api/v1/players/not-a-real-id")
    assert response.status_code == 404
