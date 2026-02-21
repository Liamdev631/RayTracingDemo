@echo off
set SAMPLES=1

cmake -S . -B build
cmake --build build --config Release

if %ERRORLEVEL% EQU 0 (

    echo Run 2: No Transparency
    build\Release\RayTracingDemo.exe --scene "scenes\glass_test.scene" --runtime 10.0 --fps 24 --width 192 --height 108 --stratified-samples %SAMPLES% --no-transparency --output "transparency_test/no_transparency.mp4"
    
    echo Run 3: Full Features
    build\Release\RayTracingDemo.exe --scene "scenes\glass_test.scene" --runtime 10.0 --fps 24 --width 192 --height 108 --stratified-samples %SAMPLES% --output "transparency_test/transparency.mp4"
) else (
    echo Build failed.
)
