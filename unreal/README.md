# Unreal Engine client

The Unreal project is generated locally with the Epic Games Launcher and will be added here after the target machine's engine version and toolchain are confirmed. Do not commit generated folders such as `Binaries/`, `Intermediate/`, `DerivedDataCache/` or `Saved/`.

## Create the project

1. Install a supported Unreal Engine 5 version through Epic Games Launcher.
2. Create a **Games → Blank → C++** desktop project.
3. Name it `FootballGame` and place it in this `unreal/` directory.
4. Enable Enhanced Input. Enable Python Editor Script Plugin only for editor automation.
5. Install Visual Studio Build Tools with the MSVC C++ toolchain and Windows SDK required by your Unreal version.
6. Generate project files and build the project once before adding gameplay code.

## Connect to the Python backend

Copy these starter files into the generated project's `Source/FootballGame/` directory:

- `StarterSource/FootballGame/FootballApiSubsystem.h`
- `StarterSource/FootballGame/FootballApiSubsystem.cpp`

In `FootballGame.Build.cs`, ensure the module dependencies include:

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

Keep any dependencies already present in your generated Build.cs. Regenerate project files and build after copying the source.

The subsystem exposes `RequestClubs`, `OnClubsReceived` and `OnRequestFailed` to Blueprint. Start the Python backend first, get the subsystem from the Game Instance, bind the two events, then call `RequestClubs`. The default API URL is `http://127.0.0.1:8000`; the URL property must not end with a slash.

This is a minimal catalogue request example, not a full gameplay/networking implementation. Do not call HTTP from Tick or use REST requests for per-frame physics.

## Content conventions

- `Content/Football/Characters`
- `Content/Football/Animations`
- `Content/Football/Ball`
- `Content/Football/Pitch`
- `Content/Football/Stadium`
- `Content/Football/UI`
- `Content/Football/Cameras`

Use original or properly licensed models, animation, audio, logos and likenesses. Large binary assets should use Git LFS.
