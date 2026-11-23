@echo off
REM ===== STATION 09: ECHO - build script (Windows) =====
REM Double-click this file. It needs a C++ compiler (MinGW-w64 g++) but NOTHING needs to be installed:
REM   - if g++ is already on your PATH it is used,
REM   - otherwise it looks for a portable "w64devkit" folder next to this file
REM     (download the .zip from https://github.com/skeeto/w64devkit/releases , unzip it here, done).
setlocal
cd /d "%~dp0"
set "CXX="
where g++ >nul 2>nul && set "CXX=g++"
if "%CXX%"=="" if exist "%~dp0w64devkit\bin\g++.exe" set "CXX=%~dp0w64devkit\bin\g++.exe"
if "%CXX%"=="" (
  echo.
  echo  No C++ compiler found.
  echo  Easiest fix: download the portable w64devkit zip from
  echo    https://github.com/skeeto/w64devkit/releases
  echo  unzip it into this folder so that  w64devkit\bin\g++.exe  exists, then run build.bat again.
  echo  ^(No installer, nothing to set up.^)  See README.md for other options.
  echo.
  pause
  exit /b 1
)
echo Building Station09.exe with %CXX% (debug symbols/runtime retained) ...
"%CXX%" -std=c++17 -O2 -mwindows -o Station09.exe src\main_win32.cpp -lgdi32 -lwinmm -luser32
if errorlevel 1 (
  echo.
  echo  BUILD FAILED - please send me the error text above.
  pause
  exit /b 1
)
echo.
echo  Done!  Run Station09.exe  ^(single file, no install needed to play^).
pause
