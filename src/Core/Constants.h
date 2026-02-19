#pragma once

/**
 * @brief Shared render constants used by the renderer and geometry.
 */
namespace Constants
{
    /** @brief Maximum number of reflection bounces before terminating a path. */
    constexpr int MAX_BOUNCE_COUNT = 5;

    /** @brief Surface offset applied to secondary rays to avoid self-intersections. */
    constexpr float SHADOW_BIAS = 0.01f;

    /** @brief Small epsilon used for numerical comparisons and stability. */
    constexpr float EPSILON = 1e-4f;

    /** @brief Enum for selecting the sampling method. */
    enum class SamplingType {
        Uniform,
        Stratified
    };

    /** @brief Default number of samples per pixel in each dimension for stratified sampling. */
    constexpr int DEFAULT_STRATIFIED_SAMPLES = 2;

    /** @brief Default sampling method configuration. */
    constexpr SamplingType DEFAULT_SAMPLING_METHOD = SamplingType::Uniform;
}
