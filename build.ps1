param(
    [string]$Config = "Release",
    [string]$BuildDir = "build",
    [string]$QtPrefix = ""
)

$ErrorActionPreference = "Stop"

if ($QtPrefix -ne "") {
    $env:CMAKE_PREFIX_PATH = $QtPrefix
}

if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
}

Write-Host "Configuring..."
cmake -S . -B $BuildDir

Write-Host "Building ($Config)..."
cmake --build $BuildDir --config $Config

Write-Host "Done."
