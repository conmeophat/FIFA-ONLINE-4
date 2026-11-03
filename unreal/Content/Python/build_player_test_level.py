"""Place a controllable footballer in the current Unreal Editor level.

Run after the FootballGame C++ module has been compiled and the stadium level
has been created. The spawned pawn auto-possesses Player 0 for a quick test.
"""

import unreal


def load_class(path):
    actor_class = unreal.load_class(None, path)
    if actor_class is None:
        raise RuntimeError(
            f"Could not load {path}. Compile the project and restart Unreal Editor."
        )
    return actor_class


def remove_labeled_actor(subsystem, label):
    for actor in subsystem.get_all_level_actors():
        if actor and actor.get_actor_label() == label:
            subsystem.destroy_actor(actor)


def main():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    character_class = load_class(
        "/Script/FootballGame.FootballPlayerCharacter"
    )
    ball_class = load_class(
        "/Script/FootballGame.FootballBallActor"
    )

    # Avoid duplicate test characters when the script is run again.
    remove_labeled_actor(subsystem, "FPO_Player_Test")
    remove_labeled_actor(subsystem, "FPO_Ball_Test")

    # The character capsule is 192 cm tall; its origin starts just above the pitch.
    player = subsystem.spawn_actor_from_class(
        character_class,
        unreal.Vector(-500.0, 0.0, 100.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if player is None:
        raise RuntimeError("Could not spawn FootballPlayerCharacter.")

    player.set_actor_label("FPO_Player_Test")
    player.set_editor_property(
        "auto_possess_player",
        unreal.AutoReceiveInput.PLAYER0,
    )

    # Start with the ball just ahead of the controlled footballer.
    ball = subsystem.spawn_actor_from_class(
        ball_class,
        unreal.Vector(-315.0, 0.0, 12.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if ball is None:
        raise RuntimeError("Could not spawn FootballBallActor.")
    ball.set_actor_label("FPO_Ball_Test")

    # The test player's spring-arm camera is now the active local view.
    # Keep the stadium's broadcast camera in the level for later use, but do
    # not let it override the possessable player camera during this test.
    for actor in subsystem.get_all_level_actors():
        if actor and actor.get_actor_label() == "FPO_Broadcast_Camera":
            try:
                actor.set_editor_property(
                    "b_activate_as_player_view_on_begin_play",
                    False,
                )
            except Exception as exc:
                unreal.log_warning(
                    f"Could not disable broadcast camera auto-activation: {exc}"
                )

    if not unreal.EditorLoadingAndSavingUtils.save_current_level():
        unreal.log_warning("Could not auto-save level. Save it manually.")

    unreal.log("Football Ball Engine v0.3: player and physics ball spawned. Player is Auto Possess Player 0.")
    unreal.log("Press Play. WASD moves relative to the player camera; mouse rotates the camera.")
    unreal.log("Hold Left Shift to sprint. Space jumps in this prototype.")
    unreal.log("E: pass | Q: through ball | Left Mouse: shoot.")


main()
