@echo off
cmake --build build --config Release
if %ERRORLEVEL% EQU 0 (
    build\Release\RayTracingDemo.exe --fps 60 --runtime 5 --width 640 --height 480
) else (
    echo Build failed.
)
