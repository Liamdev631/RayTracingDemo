#pragma once

/**
 * @brief Shared render constants used by the renderer and geometry.
 */
namespace Constants
{
    /** @brief Maximum number of reflection bounces before terminating a path. */
    constexpr int MAX_BOUNCE_COUNT = 10;

    /** @brief Surface offset applied to secondary rays to avoid self-intersections. */
    constexpr float SHADOW_BIAS = 0.01f;

    /** @brief Global reflection weight used by legacy shading paths. */
    constexpr float REFLECTION_INTENSITY = 0.3f;

    /** @brief Small epsilon used for numerical comparisons and stability. */
    constexpr float EPSILON = 1e-4f;
}
