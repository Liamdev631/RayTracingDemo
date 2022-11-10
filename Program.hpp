#pragma once
#include "SceneRenderer.h"
#include "GeometryGroup.h"
#include <thread>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <random>

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
		return;
	}

	void Render()
	{
		_window.clear(sf::Color::Magenta);
		//_renderTarget = _renderer.Render();
		//_imageBox.setTexture(_renderTarget.get());
		_renderer.Render(_renderTarget);
		_imageBox.setTexture(_renderTarget.get());

		_window.draw(_imageBox);

		return;
	}

public:
	Program()
		: _renderer(SceneRenderer({ 1920*2, 1080*2 }))
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
		_renderTarget->create(1920 * 2, 1080 * 2);
		_imageBox.setTexture(_renderTarget.get());

		// Set up the scene
		printf("Building the scene.\n");
		_scene = make_shared<Scene>();
		_renderer.SetScene(_scene);

		_scene->AmbientLightColor = { 1, 1, 1 };
		_scene->AmbientIntensity = 0.1f;

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

		// Add random spheres
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
			Render();
			_window.display();

			// Sleep a little
			//this_thread::sleep_for(100ms);
		}

		return;
	}
};
