#include <CLI/CLI.hpp>
#include <iostream>
#include "SceneRenderer.h"
#include "GeometryGroup.h"
#include <thread>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include "SceneLoader.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stb_image_write.h>
#include "AviWriter.h"
#include <cmath>

using namespace std;
using namespace chrono_literals;

const float RenderScale = std::sqrt(2.0f);
int TargetWidth = 960;
int TargetHeight = 540;
int RenderWidth = static_cast<int>(TargetWidth * RenderScale);
int RenderHeight = static_cast<int>(TargetHeight * RenderScale);

// Global variables to replace Program class state
sf::RenderWindow _window;
std::unique_ptr<SceneRenderer> _renderer; // Higher resolution for supersampling
std::shared_ptr<Scene> _scene;
sf::RectangleShape _imageBox;
unique_ptr<sf::Texture> _renderTargetHighRes;
unique_ptr<sf::Texture> _renderTargetFinal;
float _totalTime = 0.0f;

void Downscale(const sf::Image& src, sf::Image& dst)
{
    // Simple bilinear downscale
    const uint8_t* srcPixels = src.getPixelsPtr();
    std::vector<uint8_t> dstPixels(TargetWidth * TargetHeight * 4);
    
    float xRatio = (float)(RenderWidth - 1) / TargetWidth;
    float yRatio = (float)(RenderHeight - 1) / TargetHeight;
    
    for (int y = 0; y < TargetHeight; y++)
    {
        for (int x = 0; x < TargetWidth; x++)
        {
            int x_l = (int)(xRatio * x);
            int y_l = (int)(yRatio * y);
            int x_h = (int)(xRatio * x) + 1;
            int y_h = (int)(yRatio * y) + 1;
            
            float x_weight = (xRatio * x) - x_l;
            float y_weight = (yRatio * y) - y_l;
            
            auto getPixel = [&](int px, int py) {
                if (px >= RenderWidth) px = RenderWidth - 1;
                if (py >= RenderHeight) py = RenderHeight - 1;
                int idx = (px + py * RenderWidth) * 4;
                return glm::vec4(srcPixels[idx], srcPixels[idx+1], srcPixels[idx+2], srcPixels[idx+3]);
            };
            
            glm::vec4 a = getPixel(x_l, y_l);
            glm::vec4 b = getPixel(x_h, y_l);
            glm::vec4 c = getPixel(x_l, y_h);
            glm::vec4 d = getPixel(x_h, y_h);
            
            glm::vec4 pixel = glm::mix(
                glm::mix(a, b, x_weight),
                glm::mix(c, d, x_weight),
                y_weight
            );
            
            int dstIdx = (x + y * TargetWidth) * 4;
            dstPixels[dstIdx] = (uint8_t)pixel.r;
            dstPixels[dstIdx+1] = (uint8_t)pixel.g;
            dstPixels[dstIdx+2] = (uint8_t)pixel.b;
            dstPixels[dstIdx+3] = (uint8_t)pixel.a;
        }
    }
    
    dst.create(TargetWidth, TargetHeight, dstPixels.data());
}

void SaveFrame()
{
    auto img = _renderTargetFinal->copyToImage();
    
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
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
    if (_renderer) _renderer->Render(_renderTargetHighRes);
    
    // Downscale
    sf::Image highRes = _renderTargetHighRes->copyToImage();
    sf::Image finalImg;
    Downscale(highRes, finalImg);
    _renderTargetFinal->loadFromImage(finalImg);

    _imageBox.setTexture(_renderTargetFinal.get());
    _window.draw(_imageBox);
}

void SetupScene(const std::string& sceneFile)
{
    srand(static_cast<unsigned int>(time(0)));

    printf("Loading scene from %s\n", sceneFile.c_str());
    if (sceneFile.empty())
    {
        printf("Building procedural scene.\n");
        _scene = make_shared<Scene>();
        _scene->AmbientLightColor = { 1, 1, 1 };
        _scene->AmbientIntensity = 0.1f;
        _scene->SunLight = DirectionalLight(glm::normalize(fvec3(-1, -1, -1)), {1, 1, 1}, 1.0f);
        
        const int num_point_lights = 4;
        for (int i = 0; i < num_point_lights; i++)
        {
            fvec3 pos = {
                glm::linearRand<float>(-3600, 3600),
                glm::linearRand<float>(-3600, 3600),
                glm::linearRand<float>(-3600, 3600)
            };
            fvec3 color = {
                glm::linearRand<float>(0, 1),
                glm::linearRand<float>(0, 1),
                glm::linearRand<float>(0, 1)
            };
            float intensity = glm::linearRand<float>(0.5f, 1.0f);
            _scene->AddLight(new PointLight(pos, color, intensity));
        }

        const int num_spheres = 32;
        for (int i = 0; i < num_spheres; i++)
        {
            fvec3 pos = {
                glm::linearRand<float>(-1200, 1200),
                glm::linearRand<float>(-1200, 1200),
                glm::linearRand<float>(-1200, 1200) };
            float rad = glm::linearRand<float>(80, 240);
            _scene->AddGeometry(new GeometrySphere(pos, rad));
        }
        
        _scene->AddGeometry(new GeometryPlane({0,0,0}, {0,1,0}));
    }
    else
    {
        _scene = SceneLoader::LoadScene(sceneFile);
        if (!_scene)
        {
            printf("Failed to load scene, exiting.\n");
            exit(1);
        }
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

    _renderTargetHighRes = make_unique<sf::Texture>();
    _renderTargetHighRes->create(RenderWidth, RenderHeight);

    _renderTargetFinal = make_unique<sf::Texture>();
    _renderTargetFinal->create(TargetWidth, TargetHeight);
    
    _imageBox.setTexture(_renderTargetFinal.get());

    _renderer = std::make_unique<SceneRenderer>(sf::Vector2u((unsigned int)RenderWidth, (unsigned int)RenderHeight));
}

int main(int argc, char **argv)
{
    CLI::App app{"Ray Tracing Demo"};

    std::string sceneFilePath = "scenes/default.json";
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

    // Update Render dimensions based on parsed Target dimensions
    RenderWidth = static_cast<int>(TargetWidth * RenderScale);
    RenderHeight = static_cast<int>(TargetHeight * RenderScale);

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
            auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
            std::string videoFilename = "output/" + oss.str() + ".avi";

            int width = TargetWidth;
            int height = TargetHeight;
            
            AviWriter writer(videoFilename, width, height, fps);
            if (!writer.IsOpen()) {
                printf("Failed to open video file for writing: %s\n", videoFilename.c_str());
                return 1;
            }

            int totalFrames = static_cast<int>(fps * runtime);
            for (int i = 0; i < totalFrames; ++i)
            {
                if (!_window.isOpen()) break;

                // Handle window events just to keep OS happy (and allow closing)
                sf::Event ev;
                while (_window.pollEvent(ev))
                    ProcessEvent(ev);

                _totalTime = static_cast<float>(i) / fps;
                
                printf("Rendering frame %d / %d (%.1f%%)\r", i + 1, totalFrames, 100.0f * (i + 1) / totalFrames);
                fflush(stdout);

                Render();
                _window.display(); // Optional: show progress

                // Capture frame for video
                sf::Image img = _renderTargetFinal->copyToImage();
                const sf::Uint8* pixels = img.getPixelsPtr();

                // AviWriter handles RGBA (we assume alpha is ignored or writer handles it)
                // stbi_write_jpg_to_func with 4 comp writes valid JPEG? Yes.
                writer.WriteFrame(pixels, width * 4);
            }
            
            printf("\nVideo saved to %s\n", videoFilename.c_str());
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
