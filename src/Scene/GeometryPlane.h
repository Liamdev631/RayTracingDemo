#pragma once
#include "Geometry.h"

class GeometryPlane : public Geometry
{
public:
	fvec3 origin;
	fvec3 normal;

public:
	GeometryPlane(const fvec3& origin, const fvec3& normal);
	~GeometryPlane() = default;

	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	bool IntersectsAny(const Ray& ray) const override;
};

