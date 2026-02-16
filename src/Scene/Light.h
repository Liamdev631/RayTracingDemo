#pragma once
#include <glm/glm.hpp>

using namespace glm;

struct PointLight
{
    fvec3 Position;
    fvec3 Color;
    float Intensity;

    PointLight(fvec3 position, fvec3 color, float intensity)
        : Position(position), Color(color), Intensity(intensity)
    { }
};

struct DirectionalLight
{
    fvec3 Direction;
    fvec3 Color;
    float Intensity;

    DirectionalLight(fvec3 direction, fvec3 color, float intensity)
        : Direction(glm::normalize(direction)), Color(color), Intensity(intensity)
    { }
    
    // Default constructor
    DirectionalLight() 
        : Direction({0, -1, 0}), Color({1, 1, 1}), Intensity(1.0f) 
    {}
};
