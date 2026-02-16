#pragma once
#include "Geometry.h"

class GeometrySphere : public Geometry
{
protected:
	fvec3 center;
	float radius;

public:
	GeometrySphere(const fvec3& center, const float& radius) noexcept;
	~GeometrySphere() noexcept;

	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	bool IntersectsAny(const Ray& ray) const override;
};

