$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

$Python = Join-Path $ProjectRoot ".venv\Scripts\python.exe"

if (-not (Test-Path $Python)) {
    py -3 -m venv .venv
}

& $Python -m pip install --upgrade pip
if ($LASTEXITCODE -ne 0) {
    throw "Could not upgrade pip."
}

& $Python -m pip install -r backend\requirements.txt
if ($LASTEXITCODE -ne 0) {
    throw "Could not install backend requirements."
}

& $Python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
