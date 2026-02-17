@echo on
cmake -S . -B build
cmake --build build --config Release
if %ERRORLEVEL% EQU 0 (
    build\Release\RayTracingDemo.exe --scene "%~1" --fps 24 --runtime 20 --width 1920 --height 1080
) else (
    echo Build failed.
)
