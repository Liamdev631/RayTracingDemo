#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <memory>

using namespace std;

class Scene;

class SceneRenderer
{
private:
    weak_ptr<Scene> _currentScene;
    const sf::Vector2u _frameSize;

public:
    SceneRenderer(const sf::Vector2u& frameSize);
    ~SceneRenderer();

    void SetScene(shared_ptr<Scene> scene) noexcept;
    void Render(unique_ptr<sf::Texture>& renderTarget);
};

