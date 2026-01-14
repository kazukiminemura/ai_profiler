param(
    [string]$Config = "Release",
    [string]$BuildDir = "build",
    [string]$QtPrefix = "",
    [string]$Generator = "",
    [string]$MingwBin = ""
)

$ErrorActionPreference = "Stop"

if ($QtPrefix -ne "") {
    $env:CMAKE_PREFIX_PATH = $QtPrefix
}

if ($Generator -eq "" -and $QtPrefix -match "mingw") {
    $Generator = "MinGW Makefiles"
}

if ($MingwBin -ne "") {
    $env:PATH = "$MingwBin;$env:PATH"
} elseif ($Generator -eq "MinGW Makefiles") {
    $candidates = @(
        (Join-Path $QtPrefix "..\\Tools\\mingw*_64\\bin"),
        (Join-Path $QtPrefix "..\\..\\Tools\\mingw*_64\\bin")
    )
    foreach ($cand in $candidates) {
        $hits = Get-ChildItem -Path $cand -ErrorAction SilentlyContinue
        foreach ($dir in $hits) {
            if (Test-Path (Join-Path $dir.FullName "g++.exe")) {
                $env:PATH = "$($dir.FullName);$env:PATH"
                break
            }
        }
    }
}

if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
}

Write-Host "Configuring..."
if ($Generator -eq "") {
    cmake -S . -B $BuildDir
} else {
    cmake -S . -B $BuildDir -G $Generator
}

Write-Host "Building ($Config)..."
cmake --build $BuildDir --config $Config

Write-Host "Done."
