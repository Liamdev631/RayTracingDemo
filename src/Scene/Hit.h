#pragma once
#include <glm/glm.hpp>

using namespace glm;

struct Hit
{
	fvec3 Position;
	fvec3 Normal;
	float Distance;
	fvec3 Color;

	Hit& operator=(const Hit& other)
	{
		Position = other.Position;
		Normal = other.Normal;
		Distance = other.Distance;
		Color = other.Color;
		return *this;
	}
};