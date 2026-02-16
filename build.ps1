$ErrorActionPreference = "Stop"

function Get-CMakePath {
    if (Get-Command cmake -ErrorAction SilentlyContinue) {
        return "cmake"
    }
    
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $cmakePath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find **\cmake.exe
        if ($cmakePath) {
            return $cmakePath
        }
    }
    
    Write-Error "CMake not found. Please install CMake or Visual Studio with C++ CMake tools."
}

$cmake = Get-CMakePath
Write-Host "Using CMake: $cmake"

# Configure
# -DCMAKE_BUILD_TYPE=Release ensures optimizations are enabled
Write-Host "Configuring..."
& $cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
# --parallel utilizes all CPU cores for faster compilation
Write-Host "Building..."
& $cmake --build build --config Release --parallel

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful! Executable is in build/Release/RayTracingDemo.exe" -ForegroundColor Green
} else {
    Write-Error "Build failed."
}
