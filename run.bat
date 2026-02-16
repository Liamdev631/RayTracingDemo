@echo off
cmake --build build --config Release --parallel
if %ERRORLEVEL% EQU 0 (
    build\Release\RayTracingDemo.exe --scene "%~1" --fps 24 --runtime 10 --width 640 --height 480
) else (
    echo Build failed.
)
