#pragma once
#include "Geometry.h"

class GeometryTriangle : public Geometry
{
public:
	fvec3 v0, v1, v2;
	fvec3 normal;
	fvec3 color;

	GeometryTriangle(fvec3 _v0, fvec3 _v1, fvec3 _v2, fvec3 _color)
		: v0(_v0), v1(_v1), v2(_v2), color(_color)
	{
		// Compute normal using cross product of two edges
		fvec3 edge1 = v1 - v0;
		fvec3 edge2 = v2 - v0;
		normal = glm::normalize(glm::cross(edge1, edge2));
	}

	virtual bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	virtual bool IntersectsAny(const Ray& ray) const override;
};
