cmake_minimum_required(VERSION 3.14)

project(RayTracingDemo CXX)

# Set Policies to suppress warnings
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
endif()
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
# Fix for Assimp CMake Warning (dev) about ADD_CUSTOM_COMMAND
if(POLICY CMP0175)
    cmake_policy(SET CMP0175 OLD)
    set(CMAKE_POLICY_DEFAULT_CMP0175 OLD)
endif()

# Enable warnings as errors for CMake (User Request)
set(CMAKE_ERROR_ON_DEV_WARNING ON)
set(CMAKE_ERROR_ON_DEPRECATED ON)

# Suppress deprecation warnings from dependencies
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress warnings from external libraries
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# Optimization and Build Speed Configuration
# Enable Unity Build to reduce compile time by merging source files
set(CMAKE_UNITY_BUILD OFF)

# Enable Interprocedural Optimization (Link Time Optimization) if supported for performance
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO/LTO enabled")
else()
    message(STATUS "IPO/LTO not supported: ${output}")
endif()

# Automated Dependency Management via FetchContent
include(FetchContent)

# 1. GLM
message(STATUS "Fetching GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
# Force GLM to be header only
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "Build dynamic/static library" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the test programs" FORCE)

FetchContent_MakeAvailable(glm)

# If GLM is header only, it might not create a 'glm' target that we can link to easily 
# in the way some projects expect (i.e. as a library).
# But GLM CMake usually creates a 'glm' interface target if not building library.
# Let's check if 'glm' target exists, otherwise create an alias or interface.
if(NOT TARGET glm)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})
endif()

# 2. SFML
# We use the 2.6.x branch to ensure compatibility with existing code while getting updates.
# SFML 3.x introduces breaking changes that might require code refactoring.
message(STATUS "Fetching SFML...")
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(SFML_BUILD_EXAMPLES OFF)
set(SFML_BUILD_DOC_USER OFF)
set(SFML_BUILD_DOC_DEV OFF)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

# 3. nlohmann_json
message(STATUS "Fetching nlohmann_json...")
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 4. CLI11
message(STATUS "Fetching CLI11...")
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.1
)
FetchContent_MakeAvailable(CLI11)

# 5. stb
message(STATUS "Fetching stb...")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()
# stb is header only, we just need the include path
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# 6. Assimp
message(STATUS "Fetching Assimp...")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

# Disable FBX, Enable OBJ (User Request)
set(ASSIMP_BUILD_FBX_IMPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3
)

# Suppress dev warnings for Assimp to avoid CMP0175 issues crashing the build
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS_OLD ${CMAKE_SUPPRESS_DEVELOPER_WARNINGS})
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "Suppress dev warnings" FORCE)

FetchContent_MakeAvailable(assimp)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${CMAKE_SUPPRESS_DEVELOPER_WARNINGS_OLD} CACHE BOOL "Suppress dev warnings" FORCE)

# Define Sources
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.hpp")

# Define Executable
add_executable(RayTracingDemo ${SOURCES})

# Include Directories
target_include_directories(RayTracingDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/Renderer
    ${CMAKE_SOURCE_DIR}/src/Scene
    ${CMAKE_SOURCE_DIR}/src/Scene/Geometry
    ${CMAKE_SOURCE_DIR}/src/Primitives
)

# Link Libraries
# SFML 2.6.x CMake targets are usually prefixed with sfml-
# But via FetchContent/add_subdirectory, they might be SFML::Graphics etc. or sfml-graphics
# Official SFML CMake defines targets as sfml-graphics, sfml-window, sfml-system (and aliases SFML::Graphics etc.)
if(TARGET SFML::Graphics)
    target_link_libraries(RayTracingDemo PRIVATE glm::glm SFML::Graphics SFML::Window SFML::System nlohmann_json::nlohmann_json CLI11::CLI11 stb assimp::assimp)
else()
    target_link_libraries(RayTracingDemo PRIVATE glm::glm sfml-graphics sfml-window sfml-system nlohmann_json::nlohmann_json CLI11::CLI11 assimp::assimp)
endif()

# Compiler Options for Performance
if(MSVC)
    target_compile_options(RayTracingDemo PRIVATE /O2 /fp:fast)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(RayTracingDemo PRIVATE -O3 -ffast-math)
endif()

# Copy DLLs to output directory on Windows if building shared libs (default for SFML usually)
# However, for simplicity and performance, static linking is often preferred, but SFML defaults to dynamic.
# If dynamic, we need to copy DLLs.
if(WIN32)
    add_custom_command(TARGET RayTracingDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:RayTracingDemo>
        COMMENT "Copying SFML DLLs..."
    )
endif()
