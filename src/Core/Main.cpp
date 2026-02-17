#include <CLI/CLI.hpp>
#include <iostream>
#include "SceneRenderer.h"
#include <thread>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include "SceneLoader.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stb_image_write.h>
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace chrono_literals;

int TargetWidth = 960;
int TargetHeight = 540;

// Global variables to replace Program class state
sf::RenderWindow _window;
std::unique_ptr<SceneRenderer> _renderer;
std::shared_ptr<Scene> _scene;
sf::RectangleShape _imageBox;
unique_ptr<sf::Texture> _renderTargetFinal;
float _totalTime = 0.0f;



void SaveFrame()
{
    auto img = _renderTargetFinal->copyToImage();
    
    auto t = std::time(nullptr);
    struct tm tm;
    localtime_s(&tm, &t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    
    if (!std::filesystem::exists("output"))
    {
        std::filesystem::create_directory("output");
    }

    std::string filename = "output/" + oss.str() + ".png";

    int result = stbi_write_png(
        filename.c_str(), 
        img.getSize().x, 
        img.getSize().y, 
        4, 
        img.getPixelsPtr(), 
        img.getSize().x * 4
    );

    if (result)
        printf("Saved frame to %s\n", filename.c_str());
    else
        printf("Failed to save frame to %s\n", filename.c_str());
}

void ProcessEvent(const sf::Event& ev) noexcept
{
    if (ev.type == sf::Event::Closed)
    {
        _window.close();
    }
    if (ev.type == sf::Event::KeyPressed)
    {
        if (ev.key.code == sf::Keyboard::Escape)
        {
            _window.close();
        }
        else if (ev.key.code == sf::Keyboard::Space)
        {
            SaveFrame();
        }
    }
}

void UpdateScene(float time)
{
    if (!_scene) return;

    // Rotate Sun
    if (_scene->SunRotationSpeed != 0.0f)
    {
        float angle = time * _scene->SunRotationSpeed;
        // Rotate initial direction around X axis to simulate day/night cycle
        fvec3 dir = _scene->InitialSunDirection;
        dir = glm::rotateX(dir, angle);
        _scene->SunLight.Direction = glm::normalize(dir);

        // Horizon check: If direction is pointing UP (y > 0), it's below horizon -> intensity 0
        if (_scene->SunLight.Direction.y > 0)
        {
            _scene->SunLight.Intensity = 0.0f;
        }
        else
        {
            _scene->SunLight.Intensity = _scene->InitialSunIntensity;
        }
    }

    // Rotate Camera
    if (_scene->CameraRotationSpeed != 0.0f)
    {
        float angle = time * _scene->CameraRotationSpeed;
        // Rotate around CameraTarget (Y axis)
        fvec3 relativePos = _scene->InitialCameraPosition - _scene->CameraTarget;
        relativePos = glm::rotateY(relativePos, angle);
        _scene->CameraPosition = _scene->CameraTarget + relativePos;
    }
}

void Render()
{
    _window.clear(sf::Color::Magenta);
    UpdateScene(_totalTime);
    if (_renderer) _renderer->Render(_renderTargetFinal);

    _imageBox.setTexture(_renderTargetFinal.get());
    _window.draw(_imageBox);
}

void SetupScene(const std::string& sceneFile)
{
    srand(static_cast<unsigned int>(time(0)));

    if (sceneFile.empty())
    {
        printf("Error: No scene file provided.\n");
        exit(1);
    }

    printf("Loading scene from %s\n", sceneFile.c_str());
    _scene = SceneLoader::LoadScene(sceneFile);
    if (!_scene)
    {
        printf("Failed to load scene from %s, exiting.\n", sceneFile.c_str());
        exit(1);
    }

    if (_renderer) _renderer->SetScene(_scene);
}

void SetupWindow()
{
    printf("Creating the window.\n");
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    settings.depthBits = 16;
    settings.stencilBits = 0;
    _window.create(sf::VideoMode(1920, 1080), "Ray Tracing Demo", sf::Style::Default, settings);
    _window.setVerticalSyncEnabled(true);
    _window.setActive(true);

    _imageBox.setSize({ 1920, 1080 });
    _imageBox.setPosition({ 0, 0 });
    _imageBox.setFillColor(sf::Color::White);

    _renderTargetFinal = make_unique<sf::Texture>();
    _renderTargetFinal->create(TargetWidth, TargetHeight);
    
    _imageBox.setTexture(_renderTargetFinal.get());

    _renderer = std::make_unique<SceneRenderer>(sf::Vector2u((unsigned int)TargetWidth, (unsigned int)TargetHeight));
}

int main(int argc, char **argv)
{
    CLI::App app{"Ray Tracing Demo"};

    std::string sceneFilePath = "scenes/default.scene";
    int fps = 60;
    float runtime = 5.0f;
    bool singleFrame = false;

    app.add_option("-s,--scene", sceneFilePath, "Path to the scene JSON file");
    app.add_option("--fps", fps, "Frames per second for video generation")->default_val(60);
    app.add_option("--runtime", runtime, "Runtime in seconds for video generation")->default_val(5.0f);
    app.add_flag("--single-frame", singleFrame, "Run in interactive single-frame mode");
    app.add_option("--width", TargetWidth, "Output video width")->default_val(960);
    app.add_option("--height", TargetHeight, "Output video height")->default_val(540);

    CLI11_PARSE(app, argc, argv);

    try {
        SetupWindow();
        SetupScene(sceneFilePath);

        if (singleFrame)
        {
            // Interactive mode
            printf("Starting interactive mode.\n");
            sf::Event ev = sf::Event();
            sf::Clock clock;
            while (_window.isOpen())
            {
                while (_window.pollEvent(ev))
                    ProcessEvent(ev);

                float dt = clock.restart().asSeconds();
                _totalTime += dt;

                Render();
                _window.display();
            }
        }
        else
        {
            // Video generation mode
            printf("Starting video generation: %d fps, %.2f seconds.\n", fps, runtime);
            
            if (!std::filesystem::exists("output"))
                std::filesystem::create_directory("output");

            auto t = std::time(nullptr);
            struct tm tm;
            localtime_s(&tm, &t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
            std::string timestamp = oss.str();
            
            // Create a temporary directory for frames
            std::string framesDir = "output/" + timestamp + "_frames";
            std::filesystem::create_directory(framesDir);
            
            std::string videoFilename = "output/" + timestamp + ".mp4";

            int width = TargetWidth;
            int height = TargetHeight;
            
            int totalFrames = static_cast<int>(fps * runtime);
            for (int i = 0; i < totalFrames; ++i)
            {
                if (!_window.isOpen()) break;

                // Handle window events just to keep OS happy (and allow closing)
                sf::Event ev;
                while (_window.pollEvent(ev))
                    ProcessEvent(ev);

                _totalTime = static_cast<float>(i) / fps;
                
                UpdateScene(_totalTime); // Update scene state for current time
                Render();
                _window.display(); // Optional: show progress

                printf("Rendering frame %d / %d (%.1f%%)\r", i + 1, totalFrames, 100.0f * (i + 1) / totalFrames);
                fflush(stdout);

                // Capture frame for video
                sf::Image img = _renderTargetFinal->copyToImage();
                
                // Save frame as PNG
                std::ostringstream frameName;
                frameName << framesDir << "/frame_" << std::setfill('0') << std::setw(4) << i << ".png";
                std::string framePath = frameName.str();
                
                if (!stbi_write_png(framePath.c_str(), width, height, 4, img.getPixelsPtr(), width * 4))
                {
                    printf("\nFailed to save frame: %s\n", framePath.c_str());
                }
            }
            
            printf("\nFrames rendered. Stitching video with ffmpeg...\n");
            
            // Construct ffmpeg command
            // Try to use absolute path if simple 'ffmpeg' fails
            std::string ffmpegPath = "ffmpeg";
            
            // Check if ffmpeg is in path
            if (std::system("ffmpeg -version > nul 2>&1") != 0)
            {
                // Try to find it in LocalAppData
                const char* localAppData = std::getenv("LOCALAPPDATA");
                if (localAppData)
                {
                    // This is a bit of a hack, but it works for Winget installs
                    // We iterate through Microsoft\WinGet\Packages looking for ffmpeg.exe
                    try {
                        std::string basePath = std::string(localAppData) + "\\Microsoft\\WinGet\\Packages";
                        if (std::filesystem::exists(basePath))
                        {
                            for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath))
                            {
                                if (entry.path().filename() == "ffmpeg.exe")
                                {
                                    ffmpegPath = entry.path().string();
                                    printf("Found ffmpeg at: %s\n", ffmpegPath.c_str());
                                    break;
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        printf("Error searching for ffmpeg: %s\n", e.what());
                    }
                }
            }

            // ffmpeg -framerate <fps> -i <dir>/frame_%04d.png -c:v libx264 -pix_fmt yuv420p <output_file>.mp4
            std::ostringstream cmd;
            // Wrap the entire command in quotes for cmd.exe if it contains quoted arguments
            // Actually, std::system just passes the string.
            // If the executable path has spaces, it must be quoted.
            // If the arguments have spaces, they must be quoted.
            // "path to exe" "arg 1" "arg 2"
            // This structure can be problematic for cmd.exe /S /C "command" logic.
            // But usually just ensuring the exe is quoted works.
            // Let's try adding "cmd /c " prefix explicitly to control quoting behavior.
            std::ostringstream finalCmd;
#ifdef _WIN32
            finalCmd << "\"\"" << ffmpegPath << "\" -y -framerate " << fps 
                     << " -i \"" << framesDir << "/frame_%04d.png\""
                     << " -c:v libx264 -pix_fmt yuv420p \"" << videoFilename << "\"\"";
#else
            finalCmd << "\"" << ffmpegPath << "\" -y -framerate " << fps 
                     << " -i \"" << framesDir << "/frame_%04d.png\""
                     << " -c:v libx264 -pix_fmt yuv420p \"" << videoFilename << "\"";
#endif
            
            printf("Running command: %s\n", finalCmd.str().c_str());
            int ret = std::system(finalCmd.str().c_str());
            
            if (ret == 0)
            {
                printf("Video saved to %s\n", videoFilename.c_str());
                
                // Cleanup frames
                printf("Cleaning up temporary frames...\n");
                std::filesystem::remove_all(framesDir);
            }
            else
            {
                printf("ffmpeg failed with return code %d. Frames are kept in %s\n", ret, framesDir.c_str());
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
