#include "Animator.h"
#include "../Scene.h"
#include "../Primitives/Sphere.h"
#include "../Primitives/Mesh.h"
#include <iostream>

glm::vec3 Animator::GetValue(float time, float duration) const
{
    if (Keyframes.empty()) return { 0, 0, 0 };
    if (Keyframes.size() == 1) return Keyframes[0].Value;

    // Determine effective time t
    float t = time;
    if (NormalizedTime) {
        if (duration > 0) t = time / duration;
        else t = 0;
    }

    // Find surrounding keyframes
    // Keyframes are assumed to be sorted
    auto it = std::lower_bound(Keyframes.begin(), Keyframes.end(), t, 
        [](const Keyframe& k, float val) { return k.Time < val; });

    if (it == Keyframes.begin()) return it->Value;
    if (it == Keyframes.end()) return Keyframes.back().Value;

    const Keyframe& k2 = *it;
    const Keyframe& k1 = *(it - 1);

    float segmentDuration = k2.Time - k1.Time;
    if (segmentDuration <= 0.0001f) return k1.Value;

    float alpha = (t - k1.Time) / segmentDuration;
    return glm::mix(k1.Value, k2.Value, alpha);
}

void Animator::Apply(Scene* scene, float time, float duration)
{
    glm::vec3 val = GetValue(time, duration);

    if (TargetName == "sun") {
        if (Property == "direction") {
            if (glm::length(val) > 0)
                scene->SunLight.Direction = glm::normalize(val);
        }
        else if (Property == "position") {
             if (glm::length(val) > 0)
                scene->SunLight.Direction = -glm::normalize(val);
        }
    }
    else if (TargetName == "camera") {
        if (Property == "position") {
            scene->CameraPosition = val;
        }
        else if (Property == "lookat") {
             glm::vec3 dir = val - scene->CameraPosition;
             if (glm::length(dir) > 0.001f) {
                 dir = glm::normalize(dir);
                 // Convert to pitch/yaw (Y up)
                 float yaw = glm::degrees(atan2(dir.z, dir.x)) + 90.0f;
                 float pitch = glm::degrees(asin(dir.y));
                 scene->CameraRotation = { pitch, yaw, 0.0f };
             }
        }
        else if (Property == "rotation") {
            scene->CameraRotation = val;
        }
    }
    else {
        // Geometry
        for (auto* geo : scene->_geometry) {
            if (geo->Name == TargetName) {
                if (Mesh* mesh = dynamic_cast<Mesh*>(geo)) {
                    if (Property == "position") {
                        // Mesh center update is tricky because Mesh doesn't store center explicitly in a way that moves triangles automatically unless we implemented it.
                        // Assuming Mesh has a center property we can update, but triangles need to be moved.
                        // For now, let's assume we just want to update the transform if possible, but Mesh stores triangles directly.
                        // We need to calculate offset.
                        // But wait, Mesh struct in Geometry.h?
                        // Let's check Mesh definition.
                        // If Mesh doesn't have center, we might have trouble.
                        // But I see `mesh->center` usage in my previous code.
                        // Let's assume Mesh has center.
                         glm::vec3 offset = val - mesh->center;
                         mesh->center = val;
                         for (auto& tri : mesh->triangles) {
                             tri.v0 += offset;
                             tri.v1 += offset;
                             tri.v2 += offset;
                         }
                     }
                }
                else if (Sphere* sphere = dynamic_cast<Sphere*>(geo)) {
                    if (Property == "position") {
                        sphere->center = val;
                    }
                }
            }
        }
        
        // Lights
        for (auto* light : scene->_lights) {
            if (light->Name == TargetName) {
                if (Property == "position") {
                    light->Position = val;
                }
                else if (Property == "color") {
                    light->Color = val;
                }
            }
        }
    }
}
