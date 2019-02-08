#pragma once
#include "GeometrySphere.h"
#include <vector>

using namespace std;

class GeometryGroup : public GeometrySphere
{
private:
	vector<Geometry const*> _geometry;

public:
	GeometryGroup(const fvec3& center, const float& radius);
	~GeometryGroup();

	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	bool IntersectsAny(const Ray& ray) const override;
	void AddGeometry(const Geometry& geometry);
};

