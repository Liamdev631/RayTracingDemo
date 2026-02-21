@echo off
set SCENE=%~1
if "%SCENE%"=="" set SCENE=scenes\material_test_4x4.scene

set WIDTH=512
set HEIGHT=512
set SAMPLES=4
set DURATION=10.0

echo Building...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
)

echo Run: 10s Animation at 256x256
build\Release\RayTracingDemo.exe --scene "%SCENE%" --width %WIDTH% --height %HEIGHT% --stratified-samples %SAMPLES% --runtime %DURATION% %2 %3 %4 %5
