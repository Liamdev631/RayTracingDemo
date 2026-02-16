#pragma once
#include "Ray.h"
#include "Scene.h"
#include <SFML/Graphics.hpp>

class Geometry
{
public:
	virtual bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const = 0;
	virtual bool IntersectsAny(const Ray& ray) const = 0;
};

inline sf::Color Color_GLM_To_SF(fvec3 color)
{
	color = glm::clamp(color, { 0, 0, 0 }, { 1, 1, 1 });
	return sf::Color(sf::Uint8(color.r * 254.f), sf::Uint8(color.g * 254.f), sf::Uint8(color.b * 254.f));
}

inline fvec3 Color_SF_To_GLM(sf::Color c)
{
	return { float(c.r / 254), float(c.g / 254), float(c.b / 254) };
}
