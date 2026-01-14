@echo off
setlocal

set CONFIG=Release
set BUILD_DIR=build
set QT_PREFIX=

:parse
if "%~1"=="" goto endparse
if /I "%~1"=="-Config" (
  set CONFIG=%~2
  shift
  shift
  goto parse
)
if /I "%~1"=="-BuildDir" (
  set BUILD_DIR=%~2
  shift
  shift
  goto parse
)
if /I "%~1"=="-QtPrefix" (
  set QT_PREFIX=%~2
  shift
  shift
  goto parse
)
shift
goto parse
:endparse

if not "%QT_PREFIX%"=="" set CMAKE_PREFIX_PATH=%QT_PREFIX%

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Configuring...
cmake -S . -B "%BUILD_DIR%"
if errorlevel 1 exit /b 1

echo Building (%CONFIG%)...
cmake --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 exit /b 1

echo Done.
endlocal
