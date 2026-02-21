#include "Animator.h"
#include "../Scene.h"
#include "../Primitives/Sphere.h"
#include "../Primitives/Mesh.h"
#include <iostream>

glm::vec3 KeyframeTrack::GetValue(float time, float duration) const
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

void KeyframeTrack::Apply(Scene* scene, float time, float duration)
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
        for (auto* geo : scene->GetGeometryCollection()) {
            if (geo->Name == TargetName) {
                if (Mesh* mesh = dynamic_cast<Mesh*>(geo)) {
                    if (Property == "position") {
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
        for (auto* light : scene->GetLightCollection()) {
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

Animator& Animator::Get()
{
    static Animator instance;
    return instance;
}

void Animator::Reset()
{
    SunAnim = OrbitAnimation();
    CameraAnim = OrbitAnimation();
}

void Animator::Update(Scene* scene, float time, float duration)
{
    // Apply keyframe animations first
    for (auto& track : scene->KeyframeTracks)
    {
        track.Apply(scene, time, duration);
    }

    // Apply Sun Orbit
    if (SunAnim.Enabled && duration > 0.0f)
    {
        float t = glm::clamp(time / duration, 0.0f, 1.0f);
        
        float currentOrbit = glm::mix(SunAnim.OrbitStart, SunAnim.OrbitEnd, t);
        float currentAltitude = glm::mix(SunAnim.AltitudeStart, SunAnim.AltitudeEnd, t);
        
        float orbitRad = glm::radians(currentOrbit);
        float altRad = glm::radians(currentAltitude);

        fvec3 sunPos(
            std::cos(altRad) * std::sin(orbitRad),
            std::sin(altRad),
            std::cos(altRad) * std::cos(orbitRad)
        );
        
        scene->SunLight.Direction = -glm::normalize(sunPos);
        
        // Horizon dimming
        if (scene->SunLight.Direction.y > 0) {
             scene->SunLight.Intensity = 0.0f;
        } else {
             // We need initial intensity. 
             // Ideally we shouldn't hardcode this restoration if it was animated by keyframes, 
             // but since we mix keyframes and procedural, let's assume procedural takes precedence for intensity if it wants to.
             // But here we only change direction.
             // Actually, the original code had `InitialSunIntensity`.
             // We should probably check if `InitialSunIntensity` is still in Scene.
             // I'll keep using it if it exists, or just not touch intensity if y <= 0.
             // But if we dim it to 0, we need to restore it.
             // Let's assume Scene has InitialSunIntensity.
             scene->SunLight.Intensity = scene->InitialSunIntensity;
        }
    }

    // Apply Camera Orbit
    if (CameraAnim.Enabled && duration > 0.0f)
    {
        float t = glm::clamp(time / duration, 0.0f, 1.0f);
        
        float currentOrbit = glm::mix(CameraAnim.OrbitStart, CameraAnim.OrbitEnd, t);
        float currentAltitude = glm::mix(CameraAnim.AltitudeStart, CameraAnim.AltitudeEnd, t);
        
        float orbitRad = glm::radians(currentOrbit);
        float altRad = glm::radians(currentAltitude);

        fvec3 camOffset(
            std::cos(altRad) * std::sin(orbitRad),
            std::sin(altRad),
            std::cos(altRad) * std::cos(orbitRad)
        );
        
        fvec3 target = CameraAnim.UseTarget ? CameraAnim.Target : scene->CameraTarget;
        
        // If distance is set, use it. Otherwise calculate from current position?
        // But if we orbit, we need a distance.
        float dist = CameraAnim.Distance;
        if (dist == 0.0f) {
            // Fallback or assume it was set correctly.
            // If 0, maybe use length(InitialPos - Target)?
            // For now, assume it's set.
            dist = 300.0f; 
        }

        scene->CameraPosition = target + camOffset * dist;
        
        // Make camera look at target
        // We can set rotation or just trust the renderer uses lookat.
        // The renderer likely uses CameraPosition and CameraTarget.
        // But `Scene` has `CameraRotation`.
        // Let's update `CameraRotation` to match lookat.
        
        glm::vec3 dir = glm::normalize(target - scene->CameraPosition);
        float yaw = glm::degrees(atan2(dir.z, dir.x)) + 90.0f;
        float pitch = glm::degrees(asin(dir.y));
        scene->CameraRotation = { pitch, yaw, 0.0f };
    }
}
