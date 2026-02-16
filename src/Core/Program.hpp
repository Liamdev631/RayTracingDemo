#pragma once
#include "SceneRenderer.h"
#include "GeometryGroup.h"
#include <thread>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <random>
#include "SceneLoader.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stb_image_write.h>

using namespace std;
using namespace chrono_literals;

class Program
{
private:
	sf::RenderWindow _window;
	SceneRenderer _renderer;
	std::shared_ptr<Scene> _scene;

	// Interface
	sf::RectangleShape _imageBox;
	unique_ptr<sf::Texture> _renderTarget;

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
		}
		return;
	}

	void Render()
	{
		_window.clear(sf::Color::Magenta);
		_renderer.Render(_renderTarget);
		_imageBox.setTexture(_renderTarget.get());
		_window.draw(_imageBox);
		return;
	}

	void SaveFrame()
	{
		auto img = _renderTarget->copyToImage();
		
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
		std::string filename = "output/" + oss.str() + ".png";

		// Using stb_image_write as requested
		// sf::Image stores pixels as RGBA (4 channels)
		// stride_in_bytes is width * 4
		int result = stbi_write_png(
			filename.c_str(), 
			img.getSize().x, 
			img.getSize().y, 
			4, 
			img.getPixelsPtr(), 
			img.getSize().x * 4
		);

		if (result)
		{
			printf("Saved frame to %s\n", filename.c_str());
		}
		else
		{
			printf("Failed to save frame to %s\n", filename.c_str());
		}
	}

public:
	Program(const std::string& sceneFile)
		: _renderer(SceneRenderer({ 1920/2, 1080/2 })) // Reduced resolution for faster test
	{
		srand(static_cast<unsigned int>(time(0)));

		// Create the window
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

		_renderTarget = make_unique<sf::Texture>();
		_renderTarget->create(1920/2, 1080/2); // Match renderer size
		_imageBox.setTexture(_renderTarget.get());

		// Set up the scene
		printf("Loading scene from %s\n", sceneFile.c_str());
		if (sceneFile.empty())
		{
			// Fallback to procedural scene if no file provided
			printf("Building procedural scene.\n");
			_scene = make_shared<Scene>();
			_scene->AmbientLightColor = { 1, 1, 1 };
			_scene->AmbientIntensity = 0.1f;
			
			// Default procedural generation
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
			
			// Add plane as requested
			_scene->AddGeometry(new GeometryPlane({0,0,0}, {0,1,0}));
		}
		else
		{
			_scene = SceneLoader::LoadScene(sceneFile);
			if (!_scene)
			{
				printf("Failed to load scene, exiting.\n");
				return;
			}
		}

		_renderer.SetScene(_scene);

		// Start the main loop
		printf("Starting the main loop.\n");
		sf::Event ev = sf::Event();
		while (_window.isOpen())
		{
			// Poll events
			while (_window.pollEvent(ev))
				ProcessEvent(ev);

			// Render
			static int frames = 1;
			printf("Rendering frame %u.\n", frames++);
			
			auto start = std::chrono::high_resolution_clock::now();
			Render();
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;
			printf("Frame time: %.2f ms\n", elapsed.count());
			
			_window.display();
			
			// Save first frame then maybe stop or continue
			if (frames == 2) 
			{
				SaveFrame();
			}
		}

		return;
	}
};
