#pragma once
#include <glm/glm.hpp>

using namespace glm;

/**
 * @brief Point light source emitting from a single position.
 */
struct PointLight
{
    /** @brief World-space light position. */
    fvec3 Position;
    /** @brief RGB light color. */
    fvec3 Color;
    /** @brief Scalar intensity multiplier. */
    float Intensity;

    /**
     * @brief Creates a point light with position, color, and intensity.
     * @param position World-space light position.
     * @param color RGB light color.
     * @param intensity Scalar intensity multiplier.
     */
    PointLight(fvec3 position, fvec3 color, float intensity)
        : Position(position), Color(color), Intensity(intensity)
    { }
};

/**
 * @brief Directional light source simulating distant illumination.
 */
struct DirectionalLight
{
    /** @brief Normalized light direction pointing from the light to the scene. */
    fvec3 Direction;
    /** @brief RGB light color. */
    fvec3 Color;
    /** @brief Scalar intensity multiplier. */
    float Intensity;

    /**
     * @brief Creates a directional light with normalized direction.
     * @param direction World-space light direction.
     * @param color RGB light color.
     * @param intensity Scalar intensity multiplier.
     */
    DirectionalLight(fvec3 direction, fvec3 color, float intensity)
        : Direction(glm::normalize(direction)), Color(color), Intensity(intensity)
    { }
    
    // Default constructor
    /** @brief Creates a default downward-facing directional light. */
    DirectionalLight() 
        : Direction({0, -1, 0}), Color({1, 1, 1}), Intensity(1.0f) 
    {}
};
