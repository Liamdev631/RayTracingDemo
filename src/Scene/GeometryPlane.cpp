#include "GeometryPlane.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

GeometryPlane::GeometryPlane(const fvec3& origin, const fvec3& normal)
	: origin(origin), normal(normal)
{

}

bool GeometryPlane::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	//glm::intersectRayPlane(ray.position, ray.direction, this->origin, this->normal,
	return false;
}

bool GeometryPlane::IntersectsAny(const Ray& ray) const
{
	return false;
}
