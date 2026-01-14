@echo off
setlocal

set CMAKE_PREFIX_PATH=C:\Qt\6.10.1\mingw_64
set CONFIG=Release
set BUILD_DIR=build
set QT_PREFIX=
set GENERATOR=
set MINGW_BIN=

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
if /I "%~1"=="-Generator" (
  set GENERATOR=%~2
  shift
  shift
  goto parse
)
if /I "%~1"=="-MingwBin" (
  set MINGW_BIN=%~2
  shift
  shift
  goto parse
)
shift
goto parse
:endparse

if "%QT_PREFIX%"=="" if not "%CMAKE_PREFIX_PATH%"=="" set QT_PREFIX=%CMAKE_PREFIX_PATH%
if not "%QT_PREFIX%"=="" set CMAKE_PREFIX_PATH=%QT_PREFIX%

if "%GENERATOR%"=="" (
  echo %QT_PREFIX% | find /I "mingw" >nul
  if not errorlevel 1 set GENERATOR=MinGW Makefiles
)

if not "%MINGW_BIN%"=="" (
  call set "PATH=%MINGW_BIN%;%%PATH%%"
  goto mingw_done
)

if "%GENERATOR%"=="MinGW Makefiles" (
  for /f "delims=" %%D in ('dir /b /ad "%QT_PREFIX%\..\Tools\mingw*_64" 2^>nul') do (
    if exist "%QT_PREFIX%\..\Tools\%%D\bin\g++.exe" (
      call set "PATH=%QT_PREFIX%\..\Tools\%%D\bin;%%PATH%%"
      goto mingw_done
    )
  )
  for /f "delims=" %%D in ('dir /b /ad "%QT_PREFIX%\..\..\Tools\mingw*_64" 2^>nul') do (
    if exist "%QT_PREFIX%\..\..\Tools\%%D\bin\g++.exe" (
      call set "PATH=%QT_PREFIX%\..\..\Tools\%%D\bin;%%PATH%%"
      goto mingw_done
    )
  )
)
:mingw_done

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Configuring...
if "%GENERATOR%"=="" (
  cmake -S . -B "%BUILD_DIR%"
) else (
  cmake -S . -B "%BUILD_DIR%" -G "%GENERATOR%"
)
if errorlevel 1 exit /b 1

echo Building (%CONFIG%)...
cmake --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 exit /b 1

echo Done.
endlocal
