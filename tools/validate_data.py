"""Validate all local football fixture JSON files."""

from app.repository import load_clubs, load_leagues, load_players


def main() -> None:
    clubs = load_clubs()
    leagues = load_leagues()
    players = load_players()

    print("Dataset validation: OK")
    print(f"Clubs: {len(clubs)}")
    print(f"Leagues: {len(leagues)}")
    print(f"Players: {len(players)}")
    print("Note: current fixtures are synthetic, not official football data.")


if __name__ == "__main__":
    main()
