# Unreal Engine client

The Unreal project is generated locally with the Epic Games Launcher and will be added here after the target machine's engine version and toolchain are confirmed. Do not commit generated folders such as `Binaries/`, `Intermediate/`, `DerivedDataCache/` or `Saved/`.

## Create the project

1. Install a supported Unreal Engine 5 version through Epic Games Launcher.
2. Create a **Games → Blank → C++** desktop project.
3. Name it `FootballGame` and place it in this `unreal/` directory.
4. Enable Enhanced Input. Enable Python Editor Script Plugin only for editor automation.
5. Install Visual Studio Build Tools with the MSVC C++ toolchain and Windows SDK required by your Unreal version.
6. Generate project files and build the project once before adding gameplay code.

## Runtime modules

When backend connectivity is added, the game module may need Unreal modules such as `HTTP`, `Json` and `JsonUtilities`. Add dependencies in the game's `Build.cs` and implement requests in C++; do not call the Python Editor API from packaged gameplay.

## Content conventions

- `Content/Football/Characters`
- `Content/Football/Animations`
- `Content/Football/Ball`
- `Content/Football/Pitch`
- `Content/Football/Stadium`
- `Content/Football/UI`
- `Content/Football/Cameras`

Use original or properly licensed models, animation, audio, logos and likenesses. Large binary assets should use Git LFS.
