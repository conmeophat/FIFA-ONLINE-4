$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$UnrealRoot = Join-Path $RepoRoot "unreal"
$UProjects = @(Get-ChildItem -Path $UnrealRoot -Filter "*.uproject" -File -Recurse -ErrorAction SilentlyContinue)

if ($UProjects.Count -eq 0) {
    throw "No .uproject found under '$UnrealRoot'. Create the FootballGame C++ project in Unreal Editor first."
}
if ($UProjects.Count -gt 1) {
    throw "More than one .uproject found under '$UnrealRoot'. Keep one project here or edit this script to select the intended project."
}

$UProject = $UProjects[0]
$ProjectRoot = $UProject.Directory.FullName
$ProjectConfig = Get-Content -Path $UProject.FullName -Raw | ConvertFrom-Json
$RuntimeModules = @($ProjectConfig.Modules | Where-Object { $_.Type -eq "Runtime" })

if ($RuntimeModules.Count -eq 0) {
    throw "No Runtime module was found in $($UProject.Name). Create a C++ project, not a Blueprint-only project."
}

$ModuleName = $RuntimeModules[0].Name
$ModuleSource = Join-Path $ProjectRoot ("Source\" + $ModuleName)
$StarterSource = Join-Path $UnrealRoot "StarterSource\FootballGame"
$BuildCs = Join-Path $ProjectRoot ("Source\" + $ModuleName + "\" + $ModuleName + ".Build.cs")

if (-not (Test-Path $BuildCs)) {
    throw "Could not find module Build.cs at '$BuildCs'. Generate the C++ project files and check the module name."
}

New-Item -ItemType Directory -Force -Path $ModuleSource | Out-Null

$RequiredFiles = @(
    "FootballApiSubsystem.h",
    "FootballApiSubsystem.cpp",
    "FootballStadiumActor.h",
    "FootballStadiumActor.cpp",
    "FootballBroadcastCameraActor.h",
    "FootballBroadcastCameraActor.cpp",
    "FootballPlayerCharacter.h",
    "FootballPlayerCharacter.cpp",
    "FootballPlayerAnimInstance.h",
    "FootballPlayerAnimInstance.cpp",
    "FootballBallActor.h",
    "FootballBallActor.cpp",
    "FootballGameplayTypes.h",
    "AnimNotify_FootballBallContact.h",
    "AnimNotify_FootballBallContact.cpp"
)

$ApiMacro = $ModuleName.ToUpperInvariant() + "_API"
foreach ($FileName in $RequiredFiles) {
    $SourcePath = Join-Path $StarterSource $FileName
    if (-not (Test-Path $SourcePath)) {
        throw "Starter source file is missing: $SourcePath"
    }

    $Content = Get-Content -Path $SourcePath -Raw
    $Content = $Content.Replace("FOOTBALLGAME_API", $ApiMacro)
    Set-Content -Path (Join-Path $ModuleSource $FileName) -Value $Content -Encoding utf8
}

# Add runtime module dependencies inside the module constructor.
$BuildContent = Get-Content -Path $BuildCs -Raw
$DependenciesToEnsure = @("HTTP", "Json", "JsonUtilities", "InputCore", "PhysicsCore")
$DependencyPattern = 'PublicDependencyModuleNames\.AddRange\(\s*new\s+string\[\]\s*\{(?<items>[\s\S]*?)\}\s*\);'
$DependencyMatch = [regex]::Match($BuildContent, $DependencyPattern)

if ($DependencyMatch.Success) {
    $DependencyBody = $DependencyMatch.Groups["items"].Value
    $MissingDependencies = @($DependenciesToEnsure | Where-Object {
        $DependencyBody -notmatch ('"' + [regex]::Escape($_) + '"')
    })

    if ($MissingDependencies.Count -gt 0) {
        foreach ($Dependency in $MissingDependencies) {
            $DependencyBody += [Environment]::NewLine + '            "' + $Dependency + '",'
        }

        $Replacement = "PublicDependencyModuleNames.AddRange(new string[] {" +
            $DependencyBody + [Environment]::NewLine + "        });"
        $BuildContent = $BuildContent.Replace($DependencyMatch.Value, $Replacement)
    }
}
else {
    $ConstructorPattern = 'public\s+\w+\s*\([^)]*\)\s*:\s*base\s*\([^)]*\)\s*\{'
    $ConstructorMatch = [regex]::Match($BuildContent, $ConstructorPattern)

    if (-not $ConstructorMatch.Success) {
        throw "Could not locate the module constructor in $BuildCs. Add HTTP, Json, JsonUtilities and InputCore to PublicDependencyModuleNames manually."
    }

    $Addition = [Environment]::NewLine +
        '        PublicDependencyModuleNames.AddRange(new string[] { "HTTP", "Json", "JsonUtilities", "InputCore", "PhysicsCore" });' +
        [Environment]::NewLine
    $InsertAt = $ConstructorMatch.Index + $ConstructorMatch.Length
    $BuildContent = $BuildContent.Insert($InsertAt, $Addition)
}

Set-Content -Path $BuildCs -Value $BuildContent -Encoding utf8

# Install Editor Python scripts and adapt reflected class paths to the real module name.
$PythonContentDir = Join-Path $ProjectRoot "Content\Python"
New-Item -ItemType Directory -Force -Path $PythonContentDir | Out-Null

foreach ($ScriptName in @("build_stadium_level.py", "build_player_test_level.py")) {
    $SourceScript = Join-Path $UnrealRoot ("Content\Python\" + $ScriptName)
    if (-not (Test-Path $SourceScript)) {
        throw "Unreal Python builder script is missing: $SourceScript"
    }

    $PythonScript = Get-Content -Path $SourceScript -Raw
    $PythonScript = $PythonScript.Replace("/Script/FootballGame.", ("/Script/" + $ModuleName + "."))
    Set-Content -Path (Join-Path $PythonContentDir $ScriptName) -Value $PythonScript -Encoding utf8
}

# Add legacy input mappings only if the exact mapping is not already present.
# These mappings support both keyboard/mouse and common XInput-style gamepads.
$ConfigDir = Join-Path $ProjectRoot "Config"
New-Item -ItemType Directory -Force -Path $ConfigDir | Out-Null
$InputConfigPath = Join-Path $ConfigDir "DefaultInput.ini"
$InputContent = if (Test-Path $InputConfigPath) {
    Get-Content -Path $InputConfigPath -Raw
} else {
    ""
}

$InputMappings = @(
    '+AxisMappings=(AxisName="MoveForward",Scale=1.000000,Key=W)',
    '+AxisMappings=(AxisName="MoveForward",Scale=-1.000000,Key=S)',
    '+AxisMappings=(AxisName="MoveForward",Scale=1.000000,Key=Gamepad_LeftY)',
    '+AxisMappings=(AxisName="MoveRight",Scale=1.000000,Key=D)',
    '+AxisMappings=(AxisName="MoveRight",Scale=-1.000000,Key=A)',
    '+AxisMappings=(AxisName="MoveRight",Scale=1.000000,Key=Gamepad_LeftX)',
    '+AxisMappings=(AxisName="Turn",Scale=1.000000,Key=MouseX)',
    '+AxisMappings=(AxisName="Turn",Scale=1.000000,Key=Gamepad_RightX)',
    '+AxisMappings=(AxisName="LookUp",Scale=-1.000000,Key=MouseY)',
    '+AxisMappings=(AxisName="LookUp",Scale=-1.000000,Key=Gamepad_RightY)',
    '+ActionMappings=(ActionName="Sprint",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftShift)',
    '+ActionMappings=(ActionName="Sprint",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_LeftShoulder)',
    '+ActionMappings=(ActionName="Jump",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=SpaceBar)',
    '+ActionMappings=(ActionName="Jump",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Bottom)',
    '+ActionMappings=(ActionName="Pass",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=E)',
    '+ActionMappings=(ActionName="Pass",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Left)',
    '+ActionMappings=(ActionName="ThroughBall",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Q)',
    '+ActionMappings=(ActionName="ThroughBall",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Top)',
    '+ActionMappings=(ActionName="Shoot",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftMouseButton)',
    '+ActionMappings=(ActionName="Shoot",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Right)',
    '+ActionMappings=(ActionName="Brace",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftControl)',
    '+ActionMappings=(ActionName="Brace",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_RightShoulder)'
)

$MissingMappings = @($InputMappings | Where-Object { -not $InputContent.Contains($_) })
if ($MissingMappings.Count -gt 0) {
    $InputAddition = [Environment]::NewLine +
        "[/Script/Engine.InputSettings]" +
        [Environment]::NewLine +
        "; Football Player Control Engine v0.5 input mappings" +
        [Environment]::NewLine +
        ($MissingMappings -join [Environment]::NewLine) +
        [Environment]::NewLine

    Add-Content -Path $InputConfigPath -Value $InputAddition -Encoding utf8
}

Write-Host ""
Write-Host "Football Player Control Engine v0.5 source installed." -ForegroundColor Green
Write-Host "Project: $($UProject.FullName)"
Write-Host "Module:  $ModuleName"
Write-Host "Source:  $ModuleSource"
Write-Host "Player input mappings: checked/installed"
Write-Host ""
Write-Host "Next:"
Write-Host "1. Close Unreal Editor."
Write-Host "2. Regenerate project files."
Write-Host "3. Build the Development Editor target in Visual Studio."
Write-Host "4. Reopen Unreal, run build_player_test_level.py, then press Play: E pass, Q through ball, Left Mouse shoot."
Write-Host "5. Assign foot sockets and action montages, then add Football Ball Contact notifies at the strike frames."
Write-Host "6. Hold Left Control / Gamepad Right Shoulder to brace during a contest."
