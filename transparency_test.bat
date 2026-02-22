@echo off
set SAMPLES=3

cmake -S . -B build
cmake --build build --config Release

if %ERRORLEVEL% EQU 0 (

    echo Run 2: No Transparency
    build\Release\RayTracingDemo.exe --scene "scenes\glass_test.scene" --runtime 10.0 --fps 60 --width 1920 --height 1080 --stratified-samples %SAMPLES% --no-transparency --output "transparency_test/no_transparency.mp4"
    
    echo Run 3: Full Features
    build\Release\RayTracingDemo.exe --scene "scenes\glass_test.scene" --runtime 10.0 --fps 60 --width 1920 --height 1080 --stratified-samples %SAMPLES% --output "transparency_test/transparency.mp4"
) else (
    echo Build failed.
)
