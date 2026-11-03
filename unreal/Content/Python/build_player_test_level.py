"""Build a playable tactical AI football scrimmage in the current Unreal level."""

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


def spawn_ai_player(
    subsystem, ai_class, label, location, team_id, attack_sign, squad_slot
):
    player = subsystem.spawn_actor_from_class(
        ai_class,
        unreal.Vector(location[0], location[1], location[2]),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if player is None:
        raise RuntimeError(f"Could not spawn AI player {label}.")
    player.set_actor_label(label)
    player.set_editor_property("team_id", team_id)
    player.set_editor_property("attack_direction_sign", attack_sign)
    player.set_editor_property("squad_slot_index", squad_slot)
    return player


def main():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    character_class = load_class("/Script/FootballGame.FootballPlayerCharacter")
    ai_character_class = load_class("/Script/FootballGame.FootballAIPlayerCharacter")
    ball_class = load_class("/Script/FootballGame.FootballBallActor")

    labels_to_remove = [
        "FPO_Player_Test", "FPO_Ball_Test",
        "FPO_AI_T0_Left", "FPO_AI_T0_Right", "FPO_AI_T0_Forward",
        "FPO_AI_T1_Left", "FPO_AI_T1_Right", "FPO_AI_T1_Defender", "FPO_AI_T1_Forward",
    ]
    for label in labels_to_remove:
        remove_labeled_actor(subsystem, label)

    player = subsystem.spawn_actor_from_class(
        character_class,
        unreal.Vector(-500.0, 0.0, 100.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if player is None:
        raise RuntimeError("Could not spawn FootballPlayerCharacter.")
    player.set_actor_label("FPO_Player_Test")
    player.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.PLAYER0)
    player.set_editor_property("team_id", 0)
    player.set_editor_property("attack_direction_sign", 1)
    player.set_editor_property("squad_slot_index", 6)

    # Team 0 attacks +X: one central midfielder, one winger and one striker support the user.
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T0_Left",
        (250.0, -950.0, 100.0), 0, 1, 7
    )
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T0_Right",
        (450.0, 950.0, 100.0), 0, 1, 8
    )
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T0_Forward",
        (1250.0, 0.0, 100.0), 0, 1, 9
    )

    # Team 1 uses the same 4-3-3 slots but attacks toward negative X.
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T1_Left",
        (1050.0, -900.0, 100.0), 1, -1, 1
    )
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T1_Right",
        (1100.0, 900.0, 100.0), 1, -1, 3
    )
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T1_Defender",
        (1750.0, 0.0, 100.0), 1, -1, 5
    )
    spawn_ai_player(
        subsystem, ai_character_class, "FPO_AI_T1_Forward",
        (2350.0, 0.0, 100.0), 1, -1, 9
    )

    ball = subsystem.spawn_actor_from_class(
        ball_class,
        unreal.Vector(-315.0, 0.0, 12.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if ball is None:
        raise RuntimeError("Could not spawn FootballBallActor.")
    ball.set_actor_label("FPO_Ball_Test")

    for actor in subsystem.get_all_level_actors():
        if actor and actor.get_actor_label() == "FPO_Broadcast_Camera":
            try:
                actor.set_editor_property(
                    "b_activate_as_player_view_on_begin_play", False
                )
            except Exception as exc:
                unreal.log_warning(
                    f"Could not disable broadcast camera auto-activation: {exc}"
                )

    if not unreal.EditorLoadingAndSavingUtils.save_current_level():
        unreal.log_warning("Could not auto-save level. Save it manually.")

    unreal.log("Football Tactical AI v0.7: 1 human, 7 AI footballers and 1 physics ball spawned.")
    unreal.log("Team formation defaults to 4-3-3; each player has a SquadSlotIndex.")
    unreal.log("AI can select short passes, through balls, crosses, and shots from open lanes.")
    unreal.log("Tactical phases: attack, defend, transition to attack and transition to defence.")


main()
