#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Ray.h"
#include "Light.h"

using namespace std;
using namespace glm;

class Geometry;

class Scene
{
public:
    // Scene Animation Settings
    float CameraRotationSpeed = 0.0f;
    float SunRotationSpeed = 0.0f;
    
    // Camera Settings (Default)
    fvec3 CameraPosition = { 0, 0, -500 };
    fvec3 InitialCameraPosition = { 0, 0, -500 };
    fvec3 CameraTarget = { 0, 0, 0 };
    float CameraFOV = 45.0f;

    // Directional light (Sun)
    DirectionalLight SunLight;
    fvec3 InitialSunDirection = { 0, -1, 0 };
    float InitialSunIntensity = 1.0f;

    // Ambient light
    fvec3 AmbientLightColor = { 1, 1, 1 };
    float AmbientIntensity = 0.1f;

private:
    vector<Geometry*> _geometry;
    vector<PointLight*> _lights;

public:
    Scene();
    ~Scene();

    void AddGeometry(Geometry* geometry);
    void AddLight(PointLight* light);
    const vector<Geometry*>& GetGeometryCollection() const;
    const vector<PointLight*>& GetLightCollection() const;
    bool Intersects(const Ray& ray, Hit& outHit) const;
    bool IntersectsAny(const Ray& ray) const;
};

