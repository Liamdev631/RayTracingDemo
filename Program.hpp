#pragma once
#include "SceneRenderer.h"
#include "GeometryGroup.h"
#include <thread>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>

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
		: _renderer(SceneRenderer({ 512, 512 }))
	{
		// Create the window
		printf("Creating the window.\n");
		sf::ContextSettings settings;
		settings.antialiasingLevel = 2;
		settings.depthBits = 16;
		settings.stencilBits = 0;
		_window.create(sf::VideoMode(512, 512), "Ray Tracing Demo", sf::Style::Default, settings);
		_window.setVerticalSyncEnabled(true);
		_window.setActive(true);

		_imageBox.setSize({ 512, 512 });
		_imageBox.setPosition({ 0, 0 });
		_imageBox.setFillColor(sf::Color::White);

		_renderTarget = make_unique<sf::Texture>();
		_renderTarget->create(512, 512);
		_imageBox.setTexture(_renderTarget.get());

		// Set up the scene
		printf("Building the scene.\n");
		_scene = make_shared<Scene>();
		_renderer.SetScene(_scene);

		// Change scene settings
		//_scene->LightDirection = normalize(fvec3(1, -2, 1));
		//_scene->LightColor = { 1, 1, 1 };
		//_scene->LightIntensity = 1.f;

		_scene->AddLight(new PointLight({ -400, 400, -400 }, { 1, -1, 1 }, {1, 1, 1}, 5.f));

		_scene->AmbientLightColor = { 1, 1, 1 };
		_scene->AmbientIntensity = 0.2f;

		// Add a bunch of random spheres
		for (int i = 0; i < 16; i++)
		{
			fvec3 pos = {
				glm::linearRand<float>(-200, 200),
				glm::linearRand<float>(-200, 200),
				glm::linearRand<float>(-200, 200) };
			float rad = glm::linearRand<float>(20, 60);
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
