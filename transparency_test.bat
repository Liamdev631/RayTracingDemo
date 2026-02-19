@echo off
set SCENE=%~1
set WIDTH=1920
set HEIGHT=1080
set SAMPLES=1

cmake -S . -B build
cmake --build build --config Release

if %ERRORLEVEL% EQU 0 (
    echo Run 1: No Transparency, No Reflections
    build\Release\RayTracingDemo.exe --scene "%SCENE%" --width %WIDTH% --height %HEIGHT% --stratified-samples %SAMPLES% --single-frame --no-transparency --no-reflections
    
    echo Run 2: No Transparency, Reflections Enabled
    build\Release\RayTracingDemo.exe --scene "%SCENE%" --width %WIDTH% --height %HEIGHT% --stratified-samples %SAMPLES% --single-frame --no-transparency
    
    echo Run 3: Full Features
    build\Release\RayTracingDemo.exe --scene "%SCENE%" --width %WIDTH% --height %HEIGHT% --stratified-samples %SAMPLES% --single-frame
) else (
    echo Build failed.
)
