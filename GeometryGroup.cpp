#include "GeometryGroup.h"

GeometryGroup::GeometryGroup(const fvec3& center, const float& radius)
	: GeometrySphere(center, radius)
{

}

GeometryGroup::~GeometryGroup()
{

}

bool GeometryGroup::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	if (!GeometrySphere::Intersects(scene, ray, outHit))
		return false;
	for (auto iter = _geometry.begin(); iter != _geometry.end(); iter++)
		if ((*iter)->Intersects(scene, ray, outHit))
			return true;
	return false;
}

bool GeometryGroup::IntersectsAny(const Ray& ray) const
{
	return false;
}

void GeometryGroup::AddGeometry(const Geometry& geometry)
{
	_geometry.push_back(&geometry);
}
