@echo off
REM Alternative: Visual Studio / Build Tools. Run from a "Developer Command Prompt for VS".  (untested)
cd /d "%~dp0"
cl /nologo /O2 /EHsc /std:c++17 /Fe:Station09.exe src\main_win32.cpp user32.lib gdi32.lib winmm.lib /link /SUBSYSTEM:WINDOWS
if errorlevel 1 ( echo BUILD FAILED & pause & exit /b 1 )
echo Done! Run Station09.exe
pause
