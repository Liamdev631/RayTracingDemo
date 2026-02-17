#include "Group.h"

Group::Group(const fvec3& center, const float& radius)
	: Sphere(center, radius)
{

}

Group::~Group()
{

}

bool Group::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	if (!Sphere::IntersectsAny(ray))
		return false;
		
	bool hitAny = false;
	for (auto iter = _geometry.begin(); iter != _geometry.end(); iter++)
		if ((*iter)->Intersects(scene, ray, outHit))
			hitAny = true;
	return hitAny;
}

bool Group::IntersectsAny(const Ray& ray) const
{
	if (!Sphere::IntersectsAny(ray))
		return false;

	for (auto iter = _geometry.begin(); iter != _geometry.end(); iter++)
		if ((*iter)->IntersectsAny(ray))
			return true;
	return false;
}

void Group::AddGeometry(const Geometry& geometry)
{
	_geometry.push_back(&geometry);
}
