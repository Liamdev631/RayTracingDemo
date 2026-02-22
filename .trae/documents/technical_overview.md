# Ray Tracing Demo - Technical Documentation

## 1. Project Overview

**Ray Tracing Demo** is a CPU-based ray tracer written in C++17. It demonstrates fundamental computer graphics concepts such as recursive ray tracing, physically-based rendering (PBR) approximations, soft shadows, and procedural animation. The project is designed to be built with CMake and Visual Studio on Windows.

## 2. Technology Stack

*   **Language**: C++17
*   **Build System**: CMake (3.14+)
*   **Dependency Management**: CMake `FetchContent` (Automated download & build)
*   **Libraries**:
    *   **SFML (2.6.x)**: Window creation, input handling, and image display/saving.
    *   **GLM**: Mathematics library (Vectors, Matrices, Transformations).
    *   **nlohmann/json**: JSON parsing for scene and material files.
    *   **CLI11**: Command-line argument parsing.
    *   **Assimp**: 3D model loading (OBJ, FBX, etc.).
*   **External Tools**:
    *   **FFmpeg**: Used for stitching rendered frames into MP4 video files.
    *   **PowerShell**: Used for build automation scripts.

## 3. Architecture

The codebase is organized into modular components within the `src` directory:

### 3.1. Core (`src/Core`)
*   **Main.cpp**: Application entry point. Handles the main loop, window events, argument parsing, and orchestrates the rendering pipeline.
*   **SceneLoader**: Parses `.scene` JSON files to instantiate geometry, lights, and settings.
*   **MeshLoader**: Loads 3D models using Assimp and converts them into internal `Mesh` and `Triangle` structures.
*   **Constants.h**: Global configuration constants (e.g., epsilon values, default settings).

### 3.2. Renderer (`src/Renderer`)
*   **SceneRenderer**: The core rendering engine.
    *   **TraceRay**: Recursive function that casts rays into the scene.
    *   **Shading**: Computes color based on material properties (Albedo, Roughness, Metallic) and lighting.
    *   **Features**:
        *   **Stratified Sampling**: Reduces aliasing by jittering rays within pixel sub-grids.
        *   **Reflections**: Recursive reflection rays based on material roughness.
        *   **Transparency**: Refraction approximations for transparent objects (e.g., glass).
        *   **Shadows**: Ray casting towards light sources to determine occlusion.

### 3.3. Scene (`src/Scene`)
*   **Scene**: Container class that holds:
    *   **Geometry**: List of all renderable objects.
    *   **Lights**: Point lights and a directional sun light.
    *   **Camera**: Position, target, and FOV settings.
    *   **Textures**: Loaded images for materials.
*   **Material**: `PBRMaterial` struct defining physical properties:
    *   **Albedo**: Base color (Texture or Vector).
    *   **Normal Map**: For surface detail.
    *   **Roughness/Metallic**: Scalar values or texture maps.
    *   **Alpha**: Transparency control.
*   **Animation**:
    *   **Animator**: Singleton managing global animations (Sun orbit, Camera orbit).
    *   **KeyframeTrack**: Handles property interpolation (Position, Rotation) over time with looping support.

### 3.4. Primitives (`src/Primitives`)
Inheritance hierarchy for renderable objects:
*   **Geometry** (Base Interface)
    *   **Sphere**: Analytic sphere intersection.
        *   **Mesh**: Bounding volume for a collection of triangles.
    *   **Triangle**: Fundamental primitive for meshes.
    *   **Plane**: Infinite plane.
    *   **Cube**: Box primitive constructed from planes.
    *   **CheckerCircle**: Procedural textured disc.

## 4. Key Features

### 4.1. Rendering Capabilities
*   **Recursive Ray Tracing**: configurable depth for reflections and refractions.
*   **PBR Workflow**: Supports standard PBR maps (Albedo, Normal, Roughness, Metallic).
*   **Anti-Aliasing**: Stratified sampling (e.g., 2x2 or 4x4 sub-samples per pixel).
*   **Shadows**: Hard shadows from point lights and directional lights.
*   **Transparency**: Supports alpha blending and refractive effects.

### 4.2. Animation System
*   **Procedural Orbits**: Automated circular orbits for the Camera and Sun light.
    *   Controls: Orbit Angle, Altitude, Distance.
*   **Keyframe Animation**: Linear interpolation between defined keyframes for arbitrary object properties.
    *   Supports Looping and Time Normalization.

### 4.3. File Formats
*   **Scene Files (`.scene`)**: JSON-based format describing the scene graph.
    ```json
    {
      "settings": { ... },
      "objects": [ ... ],
      "lights": [ ... ],
      "animators": [ ... ]
    }
    ```
*   **Material Files (`.mat`)**: JSON-based format for reusable material definitions.

### 4.4. Video Generation
The application supports a non-interactive mode for rendering animations:
1.  Renders frames to disk as PNG files.
2.  Invokes **FFmpeg** to stitch frames into an MP4 video.
3.  Automatically cleans up temporary frame files.

## 5. Build & Deployment

### 5.1. Build Script (`build.ps1`)
A PowerShell script that automates the CMake build process:
1.  Checks for CMake availability.
2.  Configures the project in Release mode.
3.  Triggers the build using all available CPU cores.

### 5.2. Execution Script (`run.bat`)
A batch script wrapper for running the demo:
1.  Rebuilds the project (incremental).
2.  Runs the executable with specified arguments (Scene, Resolution, Samples, Runtime).
3.  Handles error reporting.
