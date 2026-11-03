$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$UnrealRoot = Join-Path $RepoRoot "unreal"
$UProjects = @(Get-ChildItem -Path $UnrealRoot -Filter "*.uproject" -File -ErrorAction SilentlyContinue)

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
    "FootballBroadcastCameraActor.cpp"
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

$BuildContent = Get-Content -Path $BuildCs -Raw
$MissingDependencies = @("HTTP", "Json", "JsonUtilities") | Where-Object {
    $BuildContent -notmatch ('"' + [regex]::Escape($_) + '"')
}

if ($MissingDependencies.Count -gt 0) {
    $DependencyNames = ($MissingDependencies | ForEach-Object { '"' + $_ + '"' }) -join ", "
    $BuildContent += [Environment]::NewLine + "// Required by the Python Football Online HTTP catalogue bridge." + [Environment]::NewLine
    $BuildContent += "PublicDependencyModuleNames.AddRange(new string[] { $DependencyNames });" + [Environment]::NewLine
    Set-Content -Path $BuildCs -Value $BuildContent -Encoding utf8
}

$PythonContentDir = Join-Path $ProjectRoot "Content\Python"
New-Item -ItemType Directory -Force -Path $PythonContentDir | Out-Null
Copy-Item -Path (Join-Path $UnrealRoot "Content\Python\build_stadium_level.py") -Destination (Join-Path $PythonContentDir "build_stadium_level.py") -Force

Write-Host ""
Write-Host "Football project source installed." -ForegroundColor Green
Write-Host "Project: $($UProject.FullName)"
Write-Host "Module:  $ModuleName"
Write-Host "Source:  $ModuleSource"
Write-Host ""
Write-Host "Next:"
Write-Host "1. Close Unreal Editor."
Write-Host "2. Regenerate project files."
Write-Host "3. Build the Development Editor target in Visual Studio."
Write-Host "4. Reopen Unreal, save an empty level, then run Content/Python/build_stadium_level.py."
