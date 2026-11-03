# Unreal Engine client

The Unreal project is generated locally with the Epic Games Launcher. This repository contains C++ starter actors for the stadium, broadcast camera and controllable player, plus Editor Python scripts. The project must be generated and compiled on the Windows development machine.

## Create the project

1. Install a supported Unreal Engine 5 version through Epic Games Launcher.
2. Create a **Games → Blank → C++** desktop project.
3. Name it FootballGame and place it in this \`unreal/\` directory.
4. Enable Python Editor Script Plugin and Editor Scripting Utilities for editor automation.
5. Install Visual Studio Build Tools with the MSVC C++ toolchain and Windows SDK required by your Unreal version.
6. Open the project once, then close Unreal Editor.

## Install the repository C++ source

From the repository root, run PowerShell:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\install_unreal_source.ps1
~~~

The script:
- Finds the Unreal \`.uproject\` recursively under \`unreal/\`.
- Copies the API bridge, stadium, broadcast camera, player character and animation instance C++ files into the generated Runtime module.
- Rewrites the API export macro and reflected Python class paths to match the module name.
- Ensures HTTP, JSON and InputCore module dependencies exist.
- Merges keyboard/mouse and common gamepad axis/action mappings into \`Config/DefaultInput.ini\`.
- Copies both Editor Python builder scripts.

Then regenerate project files and build the **Development Editor** target in Visual Studio. Reopen the project after a successful local build. The C++ source has not been compiled in this repository's cloud environment.

## Build the stadium level

1. Create and save a level.
2. Open **Tools → Execute Python Script**.
3. Run \`Content/Python/build_stadium_level.py\`.
4. The script places the pitch/stadium blockout, a tracking target, broadcast camera and directional key light.

See the [stadium and camera guide](../docs/STADIUM_BROADCAST_CAMERA.md).

## Add the controllable footballer

1. Open the stadium map.
2. Run \`Content/Python/build_player_test_level.py\` using **Tools → Execute Python Script**.
3. The script spawns \`FPO_Player_Test\` at the pitch centre and sets Auto Possess Player 0.
4. Press Play and use WASD/mouse, Left Shift and Space.
5. For detailed controls, rigged mesh import and Animation Blueprint setup, read [Football Player Engine v0.2](../docs/PLAYER_ENGINE_V0_2.md).

The footballer has a primitive mannequin proxy until a compatible Skeletal Mesh is assigned. Use only character models, kits, animations and likenesses you are licensed to use.

## Connect to the Python backend

The \`FootballApiSubsystem\` starter exposes \`RequestClubs\`, \`OnClubsReceived\` and \`OnRequestFailed\` to Blueprint. Start the Python backend first, get the subsystem from the Game Instance, bind the two events, then call \`RequestClubs\`. The default API URL is \`http://127.0.0.1:8000\`; it must not end with a slash.

HTTP is for catalogue/business data, not per-frame physics. The match simulation and real-time gameplay belong in Unreal C++/Blueprints.

## Content conventions

- \`Content/Football/Characters\`
- \`Content/Football/Animations\`
- \`Content/Football/Ball\`
- \`Content/Football/Pitch\`
- \`Content/Football/Stadium\`
- \`Content/Football/UI\`
- \`Content/Football/Cameras\`

Use original or properly licensed models, animation, audio, logos and likenesses. Large Unreal binary assets should use Git LFS.
