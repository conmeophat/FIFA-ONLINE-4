# Unreal Engine client

The Unreal project is generated locally with the Epic Games Launcher. This repository includes a procedural stadium blockout actor and a broadcast camera actor, but the project still must be generated and compiled on the Windows development machine.

## Create the project

1. Install a supported Unreal Engine 5 version through Epic Games Launcher.
2. Create a **Games → Blank → C++** desktop project.
3. Name it FootballGame and place it in this unreal/ directory.
4. Enable Python Editor Script Plugin and Editor Scripting Utilities for editor automation.
5. Install Visual Studio Build Tools with the MSVC C++ toolchain and Windows SDK required by your Unreal version.
6. Open the project once, then close Unreal Editor.

## Install the repository C++ source

From the repository root, run PowerShell:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\install_unreal_source.ps1
~~~

The script copies the API bridge, stadium actor and broadcast camera actor into the generated Runtime module, adds missing HTTP/JSON dependencies to the module Build.cs, and installs the Editor Python level-builder script. It adapts the C++ API macro to the Runtime module name in your .uproject.

Then regenerate project files and build the **Development Editor** target in Visual Studio. The code has not been compiled in this repository's cloud environment; local Unreal/MSVC compilation is required.

## Build the stadium level

1. Open Unreal and create/save a level once, for example Content/Football/Maps/L_Stadium_Blockout.
2. Open **Tools → Execute Python Script**.
3. Run Content/Python/build_stadium_level.py.
4. The script places the stadium actor, a tracking target, broadcast camera and directional key light into the current level.

See the [stadium and camera guide](../docs/STADIUM_BROADCAST_CAMERA.md) for dimensions, materials, camera tuning and the smoke test.

## Connect to the Python backend

The FootballApiSubsystem starter exposes RequestClubs, OnClubsReceived and OnRequestFailed to Blueprint. Start the Python backend first, get the subsystem from the Game Instance, bind the two events, then call RequestClubs. The default API URL is http://127.0.0.1:8000; it must not end with a slash.

HTTP is for catalogue/business data, not per-frame physics. The match simulation and real-time gameplay belong in Unreal C++/Blueprints.

## Content conventions

- Content/Football/Characters
- Content/Football/Animations
- Content/Football/Ball
- Content/Football/Pitch
- Content/Football/Stadium
- Content/Football/UI
- Content/Football/Cameras

Use original or properly licensed models, animation, audio, logos and likenesses. Large Unreal binary assets should use Git LFS.
