cmake_minimum_required(VERSION 3.14)

project(RayTracingDemo CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization and Build Speed Configuration
# Enable Unity Build to reduce compile time by merging source files
set(CMAKE_UNITY_BUILD ON)

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
FetchContent_MakeAvailable(glm)

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

# Define Sources
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.hpp")

# Define Executable
add_executable(RayTracingDemo ${SOURCES})

# Include Directories
target_include_directories(RayTracingDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/Renderer
    ${CMAKE_SOURCE_DIR}/src/Scene
)

# Link Libraries
# SFML 2.6.x CMake targets are usually prefixed with sfml-
# But via FetchContent/add_subdirectory, they might be SFML::Graphics etc. or sfml-graphics
# Official SFML CMake defines targets as sfml-graphics, sfml-window, sfml-system (and aliases SFML::Graphics etc.)
if(TARGET SFML::Graphics)
    target_link_libraries(RayTracingDemo PRIVATE glm::glm SFML::Graphics SFML::Window SFML::System nlohmann_json::nlohmann_json CLI11::CLI11 stb)
else()
    target_link_libraries(RayTracingDemo PRIVATE glm::glm sfml-graphics sfml-window sfml-system nlohmann_json::nlohmann_json CLI11::CLI11 stb)
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
