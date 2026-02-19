#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

struct Keyframe {
    float Time; // In seconds (or normalized 0-1)
    glm::vec3 Value; // Value at this time
};

class Scene;

/**
 * @brief Handles keyframe-based animation for scene objects.
 */
class Animator {
public:
    /** @brief Target object name (e.g. "sun", "camera", or geometry name). */
    std::string TargetName;
    /** @brief Property to animate (e.g. "position", "direction"). */
    std::string Property;
    /** @brief List of keyframes sorted by time. */
    std::vector<Keyframe> Keyframes;
    /** @brief If true, keyframe times are normalized [0,1] relative to runtime. */
    bool NormalizedTime = false;

    /**
     * @brief Interpolates value at given time.
     * @param time Current time in seconds.
     * @param duration Total runtime duration in seconds.
     * @return Interpolated value.
     */
    glm::vec3 GetValue(float time, float duration) const;

    /**
     * @brief Applies animation to the scene.
     * @param scene Target scene.
     * @param time Current time in seconds.
     * @param duration Total runtime duration in seconds.
     */
    void Apply(Scene* scene, float time, float duration);
};
