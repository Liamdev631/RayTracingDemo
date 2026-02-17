#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <memory>

using namespace std;

class Scene;

/**
 * @brief Renders a scene into an SFML texture using a CPU ray tracer.
 */
class SceneRenderer
{
private:
    weak_ptr<Scene> _currentScene;
    const sf::Vector2u _frameSize;

public:
    /**
     * @brief Creates a renderer for a fixed frame size.
     * @param frameSize Output resolution in pixels.
     */
    SceneRenderer(const sf::Vector2u& frameSize);
    /**
     * @brief Destroys the renderer.
     */
    ~SceneRenderer();

    /**
     * @brief Assigns the active scene to render.
     * @param scene Scene to render.
     */
    void SetScene(shared_ptr<Scene> scene) noexcept;
    /**
     * @brief Renders the scene into the target texture.
     * @param renderTarget Texture to write the rendered image into.
     */
    void Render(unique_ptr<sf::Texture>& renderTarget);

private:
    glm::fvec3 TraceRay(const Scene* scene, const Ray& ray, int depth);
};

