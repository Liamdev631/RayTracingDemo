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

// Struct to hold application state
struct AppState {
    int resolutionX = 640;
    int resolutionY = 480;
    int frameCount = 0;
    float totalTime = 0.0f;
    float targetRuntime = 10.0f;
    std::vector<double> frameTimes;

    std::unique_ptr<sf::RenderWindow> window;
    std::shared_ptr<Scene> scene;
    std::unique_ptr<SceneRenderer> renderer;
    std::unique_ptr<sf::RectangleShape> imageBox;
    std::unique_ptr<sf::Texture> renderTargetFinal;
};

void SaveFrame(AppState& app)
{
    if (!app.renderTargetFinal) return;
    auto img = app.renderTargetFinal->copyToImage();
    
    std::ostringstream oss;
    oss << "_" << std::setw(4) << std::setfill('0') << app.frameCount++;    
    
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

void ProcessEvent(AppState& app, const sf::Event& ev) noexcept
{
    if (!app.window) return;
    if (ev.type == sf::Event::Closed)
    {
        app.window->close();
    }
    if (ev.type == sf::Event::KeyPressed)
    {
        if (ev.key.code == sf::Keyboard::Escape)
        {
            app.window->close();
        }
        else if (ev.key.code == sf::Keyboard::Space)
        {
            SaveFrame(app);
        }
    }
}

void UpdateScene(AppState& app, float time, float duration)
{
    if (!app.scene) return;

    // Update Animators
    app.scene->Update(time, duration);
}

void Render(AppState& app)
{
    if (!app.window) return;
    app.window->clear(sf::Color::Magenta);
    UpdateScene(app, app.totalTime, app.targetRuntime);    
    
    auto start = std::chrono::high_resolution_clock::now();
    if (app.renderer) app.renderer->Render(app.renderTargetFinal);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    app.frameTimes.push_back(ms.count());

    if (app.imageBox && app.renderTargetFinal)
    {
        app.imageBox->setTexture(app.renderTargetFinal.get());
        app.window->draw(*app.imageBox);
    }
}

void SetupWindow(AppState& app)
{
    printf("Creating the window.\n");
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    settings.depthBits = 16;
    settings.stencilBits = 0;
    
    app.window = std::make_unique<sf::RenderWindow>(sf::VideoMode(app.resolutionX, app.resolutionY), "Ray Tracing Demo", sf::Style::Default, settings);
    app.window->setVerticalSyncEnabled(true);
    app.window->setActive(true);

    app.imageBox = std::make_unique<sf::RectangleShape>();
    app.imageBox->setSize({ (float)app.resolutionX, (float)app.resolutionY }); 
    app.imageBox->setPosition({ 0, 0 });
    app.imageBox->setFillColor(sf::Color::White);

    app.renderTargetFinal = make_unique<sf::Texture>();
    app.renderTargetFinal->create(app.resolutionX, app.resolutionY);
    
    app.imageBox->setTexture(app.renderTargetFinal.get());

    app.renderer = std::make_unique<SceneRenderer>(sf::Vector2u((unsigned int)app.resolutionX, (unsigned int)app.resolutionY));
}

int main(int argc, char** argv)
{
    std::cout << "RayTracingDemo Starting..." << std::endl;
    std::flush(std::cout);
    
    AppState appState;

    CLI::App app{ "Ray Tracing Demo" };
    
    std::string scenePath = "scenes/scene.json";
    int fps = 24;
    float runtime = 10.0f;
    bool singleFrame = false;
    bool noTransparency = false;
    bool noReflections = false;
    int stratifiedSamples = Constants::DEFAULT_STRATIFIED_SAMPLES;
    std::string outputFile = "";

    app.add_option("--scene", scenePath, "Path to scene file");
    app.add_option("--width", appState.resolutionX, "Output width");
    app.add_option("--height", appState.resolutionY, "Output height");
    app.add_option("--fps", fps, "Frames per second");
    app.add_option("--runtime", runtime, "Runtime in seconds");
    app.add_option("--stratified-samples", stratifiedSamples, "Number of stratified samples per dimension (e.g. 2 means 4 samples/pixel)");
    app.add_option("--output", outputFile, "Output video filename");
    app.add_flag("--single-frame", singleFrame, "Run in interactive single-frame mode");
    app.add_flag("--no-transparency", noTransparency, "Disable transparency");
    app.add_flag("--no-reflections", noReflections, "Disable reflections");
    
    CLI11_PARSE(app, argc, argv);

    appState.targetRuntime = (float)runtime;

    std::cout << "Loading scene: " << scenePath << std::endl;
    
    if (scenePath.empty() || !std::filesystem::exists(scenePath))
    {
        std::cerr << "Error: Scene file not found: " << scenePath << std::endl;
        return 1;
    }

    // Initialize Window and Renderer resources
    try {
        SetupWindow(appState);
        
        if (appState.renderer)
        {
            appState.renderer->SetTransparency(!noTransparency);
            appState.renderer->SetReflections(!noReflections);
            appState.renderer->SetStratifiedSamples(stratifiedSamples);
            if (stratifiedSamples > 1)
                appState.renderer->SetSamplingMethod(Constants::SamplingType::Stratified);
            else
                appState.renderer->SetSamplingMethod(Constants::SamplingType::Uniform);
        }

        try {
            appState.scene = SceneLoader::LoadScene(scenePath);
        } catch (const std::exception& e) {
            std::cerr << "Exception loading scene: " << e.what() << std::endl;
            return 1;
        }

        if (!appState.scene)
        {
            std::cerr << "Failed to load scene." << std::endl;
            return 1;
        }
        
        if (appState.renderer) appState.renderer->SetScene(appState.scene);
        
        std::cout << "Scene loaded successfully." << std::endl;

        if (singleFrame)
        {
            // Interactive mode
            printf("Starting interactive mode. Initial time: %.2fs (25%% of runtime)\n", runtime * 0.25f);
            appState.totalTime = runtime * 0.25f; // Start at 25% of runtime
            
            sf::Event ev = sf::Event();
            sf::Clock clock;
            while (appState.window && appState.window->isOpen())
            {
                while (appState.window->pollEvent(ev))
                    ProcessEvent(appState, ev);

                Render(appState);
                if (appState.window) appState.window->display();
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
            std::filesystem::create_directories(framesDir);
            
            std::string videoFilename;
            if (!outputFile.empty())
            {
                videoFilename = outputFile;
                std::filesystem::path p(videoFilename);
                if (p.has_parent_path())
                {
                    std::filesystem::create_directories(p.parent_path());
                }
            }
            else
            {
                videoFilename = "output/" + timestamp + ".mp4";
            }

            int width = appState.resolutionX;
            int height = appState.resolutionY;
            
            int totalFrames = static_cast<int>(fps * runtime);
            for (int i = 0; i < totalFrames; ++i)
            {
                if (!appState.window || !appState.window->isOpen()) break;

                // Handle window events just to keep OS happy (and allow closing)
                sf::Event ev;
                while (appState.window->pollEvent(ev))
                    ProcessEvent(appState, ev);

                appState.totalTime = static_cast<float>(i) / fps;
                
                UpdateScene(appState, appState.totalTime, runtime); // Update scene state for current time
                Render(appState);
                if (appState.window) appState.window->display(); // Optional: show progress

                printf("\rRendering frame %d / %d (%.1f%%)", i + 1, totalFrames, 100.0f * (i + 1) / totalFrames);
                fflush(stdout);

                // Capture frame for video
                sf::Image img = appState.renderTargetFinal->copyToImage();
                
                // Save frame as PNG
                std::ostringstream frameName;
                frameName << framesDir << "/frame_" << std::setfill('0') << std::setw(4) << appState.frameCount++ << ".png";
                std::string framePath = frameName.str();
                
                if (!img.saveToFile(framePath))
                {
                    printf("\nFailed to save frame: %s\n", framePath.c_str());
                }
            }
            
            printf("\nFrames rendered. Stitching video with ffmpeg...\n");
            
            // Construct ffmpeg command
            std::string ffmpegPath = "ffmpeg";
            
            // Check if ffmpeg is in path
            if (std::system("ffmpeg -version > nul 2>&1") != 0)
            {
                const char* localAppData = std::getenv("LOCALAPPDATA");
                if (localAppData)
                {
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
#ifdef _WIN32
            // Windows specific quoting
            // Use cmd /c to handle quotes properly
            cmd << "cmd /c \"\"" << ffmpegPath << "\" -y -framerate " << fps 
                << " -i \"" << framesDir << "/frame_%04d.png\""
                << " -c:v libx264 -pix_fmt yuv420p \"" << videoFilename << "\"\"";
#else
            cmd << "\"" << ffmpegPath << "\" -y -framerate " << fps 
                << " -i \"" << framesDir << "/frame_%04d.png\""
                << " -c:v libx264 -pix_fmt yuv420p \"" << videoFilename << "\"";
#endif
            
            printf("Running command: %s\n", cmd.str().c_str());
            int ret = std::system(cmd.str().c_str());
            
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

    if (!appState.frameTimes.empty())
    {
        double sum = 0.0;
        double minT = appState.frameTimes[0];
        double maxT = appState.frameTimes[0];
        for (double t : appState.frameTimes)
        {
            sum += t;
            if (t < minT) minT = t;
            if (t > maxT) maxT = t;
        }
        double mean = sum / appState.frameTimes.size();
        
        double sqSum = 0.0;
        for (double t : appState.frameTimes)
        {
            sqSum += (t - mean) * (t - mean);
        }
        double stdDev = std::sqrt(sqSum / appState.frameTimes.size());
        
        printf("\nFrame Time Stats (ms):\n");
        printf("  Count: %zu\n", appState.frameTimes.size());
        printf("  Mean:  %.2f ms\n", mean);
        printf("  Std:   %.2f ms\n", stdDev);
        printf("  Min:   %.2f ms\n", minT);
        printf("  Max:   %.2f ms\n", maxT);
    }
    
    // Explicitly reset pointers to control destruction order
    // Order: Renderer (uses Scene, Texture), Scene, Texture, Window
    // But Renderer might use Texture (renderTargetFinal).
    // Actually Render() passes renderTargetFinal to Renderer->Render().
    // Renderer does NOT own renderTargetFinal.
    // Renderer DOES own shared_ptr<Scene>.
    
    // Safe order:
    // 1. Renderer (releases scene)
    // 2. Scene (releases resources)
    // 3. ImageBox (uses Texture)
    // 4. Texture (renderTargetFinal) - depends on Window context?
    // 5. Window (must be last)
    
    printf("Cleaning up resources...\n");
    appState.renderer.reset();
    appState.scene.reset();
    appState.imageBox.reset();
    appState.renderTargetFinal.reset();
    appState.window.reset();

    printf("Exiting application.\n");
    return 0;
}
