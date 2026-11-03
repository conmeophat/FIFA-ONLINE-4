"""Build a starter football stadium blockout in the currently open Unreal level.

Run this script from Unreal Editor's Python console or Execute Python Script.
The native C++ classes must be compiled first. The map must already be saved once.
"""

import unreal


def load_native_class(path: str):
    actor_class = unreal.load_class(None, path)
    if actor_class is None:
        raise RuntimeError(
            f"Could not load {path}. Compile the FootballGame C++ project, "
            "reopen the Unreal Editor, then run this script again."
        )
    return actor_class


def destroy_actor_with_label(actor_subsystem, label: str) -> None:
    for actor in actor_subsystem.get_all_level_actors():
        if actor and actor.get_actor_label() == label:
            actor_subsystem.destroy_actor(actor)


def spawn(actor_subsystem, actor_class, label, location, rotation=None):
    actor = actor_subsystem.spawn_actor_from_class(
        actor_class,
        location,
        rotation or unreal.Rotator(),
        False,
    )
    if actor is None:
        raise RuntimeError(f"Failed to spawn actor: {label}")

    actor.set_actor_label(label)
    return actor


def main():
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    stadium_class = load_native_class("/Script/FootballGame.FootballStadiumActor")
    camera_class = load_native_class("/Script/FootballGame.FootballBroadcastCameraActor")

    # Remove this script's previous blockout actors so rerunning it is safe.
    known_labels = [
        "FPO_Stadium_Blockout",
        "FPO_Broadcast_Target",
        "FPO_Broadcast_Camera",
        "FPO_Sun_Key_Light",
    ]
    for label in known_labels:
        destroy_actor_with_label(actor_subsystem, label)

    stadium = spawn(
        actor_subsystem,
        stadium_class,
        "FPO_Stadium_Blockout",
        unreal.Vector(0.0, 0.0, 0.0),
    )

    # This target can later be replaced with the live football actor.
    tracking_target = spawn(
        actor_subsystem,
        unreal.TargetPoint,
        "FPO_Broadcast_Target",
        unreal.Vector(0.0, 0.0, 100.0),
    )

    camera = spawn(
        actor_subsystem,
        camera_class,
        "FPO_Broadcast_Camera",
        unreal.Vector(-900.0, -9900.0, 5200.0),
    )
    camera.set_editor_property("tracked_actor", tracking_target)

    # A simple key light makes the greybox readable. Add a Sky Light and
    # production lighting rig manually when the visual materials are ready.
    sun = spawn(
        actor_subsystem,
        unreal.DirectionalLight,
        "FPO_Sun_Key_Light",
        unreal.Vector(0.0, 0.0, 9000.0),
        unreal.Rotator(-45.0, -30.0, 0.0),
    )

    saved = unreal.EditorLoadingAndSavingUtils.save_current_level()
    if not saved:
        unreal.log_warning(
            "Football stadium actors were created, but the level was not saved. "
            "Save the current map manually."
        )

    unreal.log("Python Football Online: stadium and broadcast camera blockout created.")
    unreal.log(f"Stadium actor: {stadium.get_path_name()}")
    unreal.log(f"Camera actor: {camera.get_path_name()}")
    unreal.log(f"Tracking target: {tracking_target.get_path_name()}")
    unreal.log(f"Sun actor: {sun.get_path_name()}")


main()
