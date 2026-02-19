#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include "Ray.h"
#include "Light.h"
#include "Animation/Animator.h"

using namespace std;
using namespace glm;

class Geometry;

/**
 * @brief Scene container for geometry, lights, camera, and animation settings.
 *
 * The scene owns geometry, lights, and loaded textures used for PBR shading.
 */
class Scene
{
    friend class Animator;
public:
    /** @brief Camera rotation speed in radians per second. */
    float CameraRotationSpeed = 0.0f;
    /** @brief Sun rotation speed in radians per second. */
    float SunRotationSpeed = 0.0f;
    
    /** @brief Current camera position in world space. */
    fvec3 CameraPosition = { 0, 0, -500 };
    /** @brief Initial camera position used for animation. */
    fvec3 InitialCameraPosition = { 0, 0, -500 };
    /** @brief Camera look-at target in world space. */
    fvec3 CameraTarget = { 0, 0, 0 };
    /** @brief Camera field of view in degrees. */
    float CameraFOV = 45.0f;

    /** @brief Directional light representing the sun. */
    DirectionalLight SunLight;
    /** @brief Initial sun orbit angle in degrees. */
    float SunOrbitStart = 0.0f;
    float SunOrbitEnd = 0.0f;
    /** @brief Sun altitude angle in degrees. */
    float SunAltitudeStart = 45.0f;
    float SunAltitudeEnd = 45.0f;
    /** @brief Initial sun intensity used for animation. */
    float InitialSunIntensity = 1.0f;

    /** @brief Ambient light color used for global illumination. */
    fvec3 AmbientLightColor = { 1, 1, 1 };
    /** @brief Ambient light intensity scalar. */
    float AmbientIntensity = 0.1f;

    /** @brief Animators attached to the scene. */
    std::vector<Animator> Animators;

    /** @brief Camera rotation in degrees (pitch, yaw, roll). */
    fvec3 CameraRotation = { 0, 0, 0 };

    /**
     * @brief Updates scene animations.
     * @param time Current time in seconds.
     * @param duration Total duration in seconds.
     */
    void Update(float time, float duration);

private:
    vector<Geometry*> _geometry;
    vector<PointLight*> _lights;
    vector<shared_ptr<sf::Image>> _textures;

public:
    /**
     * @brief Initializes a scene with default settings and empty collections.
     */
    Scene();
    /**
     * @brief Releases scene resources and deletes owned geometry and lights.
     */
    ~Scene();
    /**
     * @brief Adds geometry to the scene.
     * @param geometry Owned geometry pointer.
     */
    void AddGeometry(Geometry* geometry);
    /**
     * @brief Adds a point light to the scene.
     * @param light Owned light pointer.
     */
    void AddLight(PointLight* light);
    /**
     * @brief Loads and stores a texture by file path.
     * @param filepath Path to the image file.
     * @return Index of the loaded texture or -1 on failure.
     */
    int AddTexture(const string& filepath);
    /**
     * @brief Returns a texture by index.
     * @param index Texture index.
     * @return Pointer to the texture or nullptr if invalid.
     */
    const sf::Image* GetTexture(int index) const;
    /**
     * @brief Returns the scene geometry collection.
     * @return Immutable list of geometry pointers.
     */
    const vector<Geometry*>& GetGeometryCollection() const;
    /**
     * @brief Returns the scene light collection.
     * @return Immutable list of light pointers.
     */
    const vector<PointLight*>& GetLightCollection() const;
    /**
     * @brief Finds the closest intersection for a ray.
     * @param ray Ray to test.
     * @param outHit Populated with the closest hit on success.
     * @return True if an intersection occurs.
     */
    bool Intersects(const Ray& ray, Hit& outHit) const;
    /**
     * @brief Tests if any geometry is hit by the ray.
     * @param ray Ray to test.
     * @return True if any geometry is hit.
     */
    bool IntersectsAny(const Ray& ray) const;
};

