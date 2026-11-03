"""Build a complete 11v11 tactical match test level."""

import unreal

FORMATION_433 = [
    (-4350.0, 0.0),     # 0 GK
    (-2600.0, -1700.0), # 1 RB
    (-2850.0, -550.0),  # 2 RCB
    (-2850.0, 550.0),   # 3 LCB
    (-2600.0, 1700.0),  # 4 LB
    (-1250.0, 0.0),     # 5 DM
    (-650.0, -750.0),   # 6 CM
    (-650.0, 750.0),    # 7 CM
    (600.0, -1550.0),   # 8 RW
    (1250.0, 0.0),      # 9 ST
    (600.0, 1550.0),    # 10 LW
]


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
    subsystem, ai_class, label, position, team_id, attack_sign, squad_slot
):
    local_x, y = position
    x = local_x * attack_sign
    player = subsystem.spawn_actor_from_class(
        ai_class,
        unreal.Vector(x, y, 100.0),
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
    match_class = load_class("/Script/FootballGame.FootballMatchDirector")

    labels_to_remove = ["FPO_Player_Test", "FPO_Ball_Test", "FPO_Match_Director"]
    for team_id in (0, 1):
        for slot in range(11):
            labels_to_remove.append(f"FPO_AI_T{team_id}_S{slot:02d}")
    labels_to_remove.extend([
        "FPO_AI_T0_Left", "FPO_AI_T0_Right", "FPO_AI_T0_Forward",
        "FPO_AI_T1_Left", "FPO_AI_T1_Right", "FPO_AI_T1_Defender", "FPO_AI_T1_Forward",
    ])
    for label in labels_to_remove:
        remove_labeled_actor(subsystem, label)

    human_slot = 6
    human_x, human_y = FORMATION_433[human_slot]
    player = subsystem.spawn_actor_from_class(
        character_class,
        unreal.Vector(human_x, human_y, 100.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if player is None:
        raise RuntimeError("Could not spawn FootballPlayerCharacter.")
    player.set_actor_label("FPO_Player_Test")
    player.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.PLAYER0)
    player.set_editor_property("team_id", 0)
    player.set_editor_property("attack_direction_sign", 1)
    player.set_editor_property("squad_slot_index", human_slot)

    # Team 0: one human plus ten AI footballers.
    for slot, position in enumerate(FORMATION_433):
        if slot == human_slot:
            continue
        spawn_ai_player(
            subsystem, ai_character_class, f"FPO_AI_T0_S{slot:02d}",
            position, 0, 1, slot
        )

    # Team 1: eleven AI footballers attacking toward negative X.
    for slot, position in enumerate(FORMATION_433):
        spawn_ai_player(
            subsystem, ai_character_class, f"FPO_AI_T1_S{slot:02d}",
            position, 1, -1, slot
        )

    ball = subsystem.spawn_actor_from_class(
        ball_class,
        unreal.Vector(0.0, 0.0, 13.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if ball is None:
        raise RuntimeError("Could not spawn FootballBallActor.")
    ball.set_actor_label("FPO_Ball_Test")

    match = subsystem.spawn_actor_from_class(
        match_class,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
        False,
    )
    if match is None:
        raise RuntimeError("Could not spawn FootballMatchDirector.")
    match.set_actor_label("FPO_Match_Director")
    match.set_editor_property("match_clock_rate", 30.0)
    match.set_editor_property("half_time_break_seconds", 8.0)

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

    unreal.log("Football Match Engine v0.9: 11v11 match with player duels, goalkeeper catch/hold and set-piece AI.")
    unreal.log("Both teams use the 4-3-3 default with goalkeeper slots at SquadSlotIndex 0.")
    unreal.log("Match Director handles clock, score, kick-offs, throw-ins, corners, goal kicks, free kicks and offside.")
    unreal.log("Goalkeepers position, parry, catch/hold the ball, and distribute with a physical long kick.")
    unreal.log("Native match HUD automatically displays team score, match clock and half/full-time state.")
    unreal.log("Shoulder-duel state is exposed to the Animation Blueprint; assign licensed reaction and contact clips for final motion.")
    unreal.log("MatchClockRate=30: one game-minute passes in two real seconds; adjust FPO_Match_Director to change pacing.")


main()
