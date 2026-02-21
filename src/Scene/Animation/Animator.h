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
class KeyframeTrack {
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

struct OrbitAnimation {
    bool Enabled = false;
    float OrbitStart = 0.0f;
    float OrbitEnd = 0.0f;
    float AltitudeStart = 45.0f;
    float AltitudeEnd = 45.0f;
    
    // For camera only
    float Distance = 0.0f;
    glm::vec3 Target = { 0, 0, 0 };
    bool UseTarget = false;
};

/**
 * @brief Singleton class that controls scene animations.
 */
class Animator {
public:
    static Animator& Get();

    OrbitAnimation SunAnim;
    OrbitAnimation CameraAnim;

    void Reset();
    void Update(Scene* scene, float time, float duration);

private:
    Animator() = default;
};
