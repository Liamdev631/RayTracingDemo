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
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace chrono_literals;

int resolutionX = 640;
int resolutionY = 480;
int frameCount = 0;

// Global variables to replace Program class state
std::unique_ptr<sf::RenderWindow> _window;
std::unique_ptr<SceneRenderer> _renderer;
std::shared_ptr<Scene> _scene;
std::unique_ptr<sf::RectangleShape> _imageBox;
std::unique_ptr<sf::Texture> _renderTargetFinal;
float _totalTime = 0.0f;
float _targetRuntime = 10.0f;
std::vector<double> _frameTimes;

void SaveFrame()
{
    if (!_renderTargetFinal) return;
    auto img = _renderTargetFinal->copyToImage();
    
    std::ostringstream oss;
    oss << "_" << std::setw(4) << std::setfill('0') << frameCount++;    
    
    if (!std::filesystem::exists("output"))
    {
        std::filesystem::create_directory("output");
    }

    std::string filename = "output/" + oss.str() + ".png";

    if (img.saveToFile(filename))
        printf("Saved frame to %s\n", filename.c_str());
    else
        printf("Failed to save frame to %s\n", filename.c_str());
}

void ProcessEvent(const sf::Event& ev) noexcept
{
    if (!_window) return;
    if (ev.type == sf::Event::Closed)
    {
        _window->close();
    }
    if (ev.type == sf::Event::KeyPressed)
    {
        if (ev.key.code == sf::Keyboard::Escape)
        {
            _window->close();
        }
        else if (ev.key.code == sf::Keyboard::Space)
        {
            SaveFrame();
        }
    }
}

void UpdateScene(float time, float duration)
{
    if (!_scene) return;

    // Update Animators
    _scene->Update(time, duration);

    // Rotate Sun (Legacy / Procedural fallback)
    if (_scene->SunRotationSpeed != 0.0f)
    {
        // Orbit around Y axis (on XZ plane)
        // SunRotationSpeed is in radians/second
        float currentOrbitRad = glm::radians(_scene->SunOrbitStart) + time * _scene->SunRotationSpeed;
        float altitudeRad = glm::radians(_scene->SunAltitudeStart);

        // Calculate Sun Position (Y is Up)
        fvec3 sunPos(
            std::cos(altitudeRad) * std::sin(currentOrbitRad),
            std::sin(altitudeRad),
            std::cos(altitudeRad) * std::cos(currentOrbitRad)
        );

        // Direction is from Sun to Origin
        _scene->SunLight.Direction = -glm::normalize(sunPos);

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
    if (!_window) return;
    _window->clear(sf::Color::Magenta);
    UpdateScene(_totalTime, _targetRuntime);    
    
    auto start = std::chrono::high_resolution_clock::now();
    if (_renderer) _renderer->Render(_renderTargetFinal);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    _frameTimes.push_back(ms.count());

    if (_imageBox && _renderTargetFinal)
    {
        _imageBox->setTexture(_renderTargetFinal.get());
        _window->draw(*_imageBox);
    }
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
    
    _window = std::make_unique<sf::RenderWindow>(sf::VideoMode(resolutionX, resolutionY), "Ray Tracing Demo", sf::Style::Default, settings);
    _window->setVerticalSyncEnabled(true);
    _window->setActive(true);

    _imageBox = std::make_unique<sf::RectangleShape>();
    _imageBox->setSize({ (float)resolutionX, (float)resolutionY }); 
    _imageBox->setPosition({ 0, 0 });
    _imageBox->setFillColor(sf::Color::White);

    _renderTargetFinal = make_unique<sf::Texture>();
    _renderTargetFinal->create(resolutionX, resolutionY);
    
    _imageBox->setTexture(_renderTargetFinal.get());

    _renderer = std::make_unique<SceneRenderer>(sf::Vector2u((unsigned int)resolutionX, (unsigned int)resolutionY));
}

int main(int argc, char** argv)
{
    std::cout << "RayTracingDemo Starting..." << std::endl;
    std::flush(std::cout);
    CLI::App app{ "Ray Tracing Demo" };
    
    std::string scenePath = "scenes/scene.json";
    int fps = 24;
    float runtime = 10.0f;
    bool singleFrame = false;
    bool noTransparency = false;
    bool noReflections = false;
    int stratifiedSamples = Constants::DEFAULT_STRATIFIED_SAMPLES;

    app.add_option("--scene", scenePath, "Path to scene file");
    app.add_option("--width", resolutionX, "Output width");
    app.add_option("--height", resolutionY, "Output height");
    app.add_option("--fps", fps, "Frames per second");
    app.add_option("--runtime", runtime, "Runtime in seconds");
    app.add_option("--stratified-samples", stratifiedSamples, "Number of stratified samples per dimension (e.g. 2 means 4 samples/pixel)");
    app.add_flag("--single-frame", singleFrame, "Run in interactive single-frame mode");
    app.add_flag("--no-transparency", noTransparency, "Disable transparency");
    app.add_flag("--no-reflections", noReflections, "Disable reflections");
    
    CLI11_PARSE(app, argc, argv);

    _targetRuntime = (float)runtime;

    std::cout << "Loading scene: " << scenePath << std::endl;
    
    if (scenePath.empty() || !std::filesystem::exists(scenePath))
    {
        std::cerr << "Error: Scene file not found: " << scenePath << std::endl;
        return 1;
    }

    // Initialize Window and Renderer resources
    try {
        SetupWindow();
        
        if (_renderer)
        {
            _renderer->SetTransparency(!noTransparency);
            _renderer->SetReflections(!noReflections);
            _renderer->SetStratifiedSamples(stratifiedSamples);
            if (stratifiedSamples > 1)
                _renderer->SetSamplingMethod(Constants::SamplingType::Stratified);
            else
                _renderer->SetSamplingMethod(Constants::SamplingType::Uniform);
        }

        try {
            _scene = SceneLoader::LoadScene(scenePath);
        } catch (const std::exception& e) {
            std::cerr << "Exception loading scene: " << e.what() << std::endl;
            return 1;
        }

    if (!_scene)
    {
        std::cerr << "Failed to load scene." << std::endl;
        return 1;
    }
    
    if (_renderer) _renderer->SetScene(_scene);
    
    std::cout << "Scene loaded successfully." << std::endl;

    // Logic continues below (reusing existing code structure where possible)
    if (singleFrame)
    {
        // Interactive mode
        printf("Starting interactive mode. Initial time: %.2fs (25%% of runtime)\n", runtime * 0.25f);
        _totalTime = runtime * 0.25f; // Start at 25% of runtime
        
        sf::Event ev = sf::Event();
        sf::Clock clock;
        while (_window && _window->isOpen())
        {
            while (_window->pollEvent(ev))
                ProcessEvent(ev);

            // Don't accumulate time in single-frame mode unless we want it to animate?
            // "If we are in --single-frame mode, we need to set the sun to 25% of that track."
            // Assuming it should stay static at that point.
            // But if it's interactive, maybe user wants to see animation?
            // Usually single-frame implies static. But the loop suggests interactive.
            // Let's assume static for now, or just let time run but start at 25%.
            // Given "set the sun to 25% of that track", it implies a fixed point.
            // But if I let it run, it will move away from 25%.
            // Let's keep it static.
            
            // float dt = clock.restart().asSeconds();
            // _totalTime += dt;

            Render();
            if (_window) _window->display();
        }
    }
    else
    {
        // Video generation mode
        printf("Starting video generation: %d fps, %.2f seconds.\n", fps, (double)runtime);
            
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

            int width = resolutionX;
            int height = resolutionY;
            
            int totalFrames = static_cast<int>(fps * runtime);
            for (int i = 0; i < totalFrames; ++i)
            {
                if (!_window || !_window->isOpen()) break;

                // Handle window events just to keep OS happy (and allow closing)
                sf::Event ev;
                while (_window->pollEvent(ev))
                    ProcessEvent(ev);

                _totalTime = static_cast<float>(i) / fps;
                
                UpdateScene(_totalTime, runtime); // Update scene state for current time
                Render();
                if (_window) _window->display(); // Optional: show progress

                printf("\rRendering frame %d / %d (%.1f%%)", i + 1, totalFrames, 100.0f * (i + 1) / totalFrames);
                fflush(stdout);

                // Capture frame for video
                sf::Image img = _renderTargetFinal->copyToImage();
                
                // Save frame as PNG
                std::ostringstream frameName;
                frameName << framesDir << "/frame_" << std::setfill('0') << std::setw(4) << frameCount++ << ".png";
                std::string framePath = frameName.str();
                
                if (!img.saveToFile(framePath))
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

    if (!_frameTimes.empty())
    {
        double sum = 0.0;
        double minT = _frameTimes[0];
        double maxT = _frameTimes[0];
        for (double t : _frameTimes)
        {
            sum += t;
            if (t < minT) minT = t;
            if (t > maxT) maxT = t;
        }
        double mean = sum / _frameTimes.size();
        
        double sqSum = 0.0;
        for (double t : _frameTimes)
        {
            sqSum += (t - mean) * (t - mean);
        }
        double stdDev = std::sqrt(sqSum / _frameTimes.size());
        
        printf("\nFrame Time Stats (ms):\n");
        printf("  Count: %zu\n", _frameTimes.size());
        printf("  Mean:  %.2f ms\n", mean);
        printf("  Std:   %.2f ms\n", stdDev);
        printf("  Min:   %.2f ms\n", minT);
        printf("  Max:   %.2f ms\n", maxT);
    }

    return 0;
}
