@echo off
set SCENE=%~1
if "%SCENE%"=="" set SCENE=scenes\glass_test.scene

set WIDTH=256
set HEIGHT=256
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
