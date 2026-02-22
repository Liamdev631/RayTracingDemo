# Ray Tracing Demo

A simple CPU-based Ray Tracing demonstration using C++, SFML, and GLM.

## Prerequisites

- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **CMake** (3.14 or later) - Included with VS 2022 or install separately.
- **PowerShell**

## Building the Project

This project uses CMake for build configuration and dependency management (FetchContent).

1.  Open a PowerShell terminal in the project root directory.
2.  Run the automated build script:

    ```powershell
    .\build.ps1
    ```

    This script will:
    - Locate your CMake installation (from PATH or Visual Studio).
    - Configure the project (Release mode).
    - Download and build dependencies (SFML, GLM).
    - Compile the application using all available CPU cores.

3.  The executable will be located at:
    `build/Release/RayTracingDemo.exe`

## Project Structure

- `src/Core`: Main entry point and application logic.
- `src/Renderer`: Rendering pipeline and scene management.
- `src/Scene`: Geometric primitives, lights, and ray definitions.
- `build.ps1`: Automated build script.
- `CMakeLists.txt`: Build configuration.

## Sample Output

### Glass Test Scene
Below is a video demonstration of the `glass_test` scene showcasing the three sample materials: Stained Glass, Ceramic, and Pool Tile. Click the image to watch on YouTube.

[![Glass Test Video](https://img.youtube.com/vi/UvUhHd8LImA/0.jpg)](https://www.youtube.com/watch?v=UvUhHd8LImA)
