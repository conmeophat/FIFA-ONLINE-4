# Football Stadium 3D + Broadcast Camera (Blockout v0.1)

This milestone adds real Unreal C++ actors to the repository. It creates a scale-aware pitch, markings, two goal frames, corner flags, simple stand tiers, floodlight masts and a smooth high sideline camera.

**Scope:** this is a playable-development blockout, not a photoreal finished stadium. Seat meshes, net simulation, crowd, licensed club branding, grass shaders, player/ball actors and a full lighting art pass are later work.

## Source files

Copy all files from `unreal/StarterSource/FootballGame/` into the generated Unreal project's `Source/FootballGame/`:

- `FootballApiSubsystem.h/.cpp`
- `FootballStadiumActor.h/.cpp`
- `FootballBroadcastCameraActor.h/.cpp`

The project module must be named `FootballGame` for the `FOOTBALLGAME_API` macro and the Python class paths below. If you use another module name, update the API macro and class paths to match.

## 1. Build the C++ project

In `FootballGame.Build.cs`, preserve the default dependencies and ensure these modules exist:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "HTTP",
    "Json",
    "JsonUtilities"
});
```

The stadium and camera actors use the Engine module. The existing Python API bridge uses HTTP and JSON modules.

Close Unreal Editor before adding new C++ files. Regenerate project files (right-click the `.uproject` → Generate project files, if available), build the Development Editor target in Visual Studio, then reopen the project.

## 2. Create a level and build the stadium

1. Open the Unreal project.
2. Create or open a level and save it once, for example `Content/Football/Maps/L_Stadium_Blockout`.
3. Enable **Python Editor Script Plugin** and **Editor Scripting Utilities**, then restart the editor.
4. Copy `unreal/Content/Python/build_stadium_level.py` into the project at `Content/Python/build_stadium_level.py`.
5. Open **Tools → Execute Python Script** and run that file.
6. Save the level if Unreal reports that the map was not saved.

The script spawns the native C++ stadium actor, a tracking Target Point, the broadcast camera and a directional key light. It removes actors created by a previous run of the same script before rebuilding, to avoid duplicate blockouts.

Unreal's Editor Python API is used here only to automate level setup. It is not used for packaged-game runtime gameplay. See Epic's [Editor Python scripting documentation](https://dev.epicgames.com/documentation/unreal-engine/scripting-the-unreal-editor-using-python).

## 3. Geometry and scale

The stadium actor uses centimetres and centres the pitch at its own origin:

| Element | Default size |
|---|---:|
| Pitch | 105 m × 68 m |
| Penalty area depth | 16.5 m |
| Penalty area width | 40.32 m |
| Goal frame | 7.32 m × 2.44 m |
| Centre circle radius | 9.15 m |

Axes: X = pitch length, Y = pitch width, Z = height. The pitch surface's top is at Z=0.

The blockout uses Unreal's built-in Cube static mesh, so no external asset is needed for the first build. Assign your own materials to the stadium actor in the Details panel:

- **Pitch Material:** green grass material.
- **Marking Material:** white line material.
- **Goal Material:** white goal / floodlight panel material.
- **Structure Material:** concrete or seat-blockout material.

To create basic materials: Content Browser → Add → Material. Connect a Constant3Vector (or Vector Parameter) to Base Color and set roughness around 0.7 for grass/stands. Save and assign each material to the corresponding exposed property.

## 4. Broadcast camera

The camera is positioned outside one touchline at a high angle and looks toward the tracked actor. It pans along the pitch length with interpolation and clamps the focus to pitch bounds.

Default tuning:

- Height: 52 m.
- Distance outside touchline: 65 m.
- Field of view: 42 degrees.
- Position smoothing: 2.8.
- Rotation smoothing: 4.0.

In the World Outliner, select `FPO_Broadcast_Camera` and set **Tracked Actor** to your ball actor when one exists. Until then, the script links it to `FPO_Broadcast_Target`. Move that Target Point during preview to test camera tracking. On Play, the camera sets itself as the Player Controller's view target.

The camera is deliberately a one-sideline broadcast rig for this first slice. A dynamic director camera, far-side cut, replay system and ball-aware zoom will be separate upgrades.

## 5. Smoke test

- The pitch is visible at the centre of the world.
- The outer lines, halfway line, centre circle, penalty areas and two goals appear.
- Three-tier stands and four floodlight masts appear when their checkboxes are enabled.
- The camera looks at `FPO_Broadcast_Target`.
- Move the target point in the editor and press Play; the camera should smoothly pan along X.

If the C++ classes cannot be loaded by the Python script, compile the project and restart Unreal Editor first. The script expects native class paths `/Script/FootballGame.FootballStadiumActor` and `/Script/FootballGame.FootballBroadcastCameraActor`.

## Known limitations

- Geometry is a greybox made from repeated built-in cubes, not a final art asset.
- Goal nets, seats, crowd, scoreboard, tunnel, benches, billboards, material textures and baked lighting are not included.
- The goals are simple frame colliders, not a complete net/goal detection system.
- C++ compilation and visual verification must be performed in the local Unreal installation; GitHub Actions currently validates only the Python backend.
