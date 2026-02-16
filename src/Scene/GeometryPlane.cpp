#include "GeometryPlane.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

GeometryPlane::GeometryPlane(const fvec3& origin, const fvec3& normal)
	: origin(origin), normal(normal)
{

}

bool GeometryPlane::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	float distance;
	if (glm::intersectRayPlane(ray.position, ray.direction, origin, normal, distance))
	{
		if (distance < 0) return false;
		
		outHit.Distance = distance;
		outHit.Position = ray.position + ray.direction * distance;
		outHit.Normal = normal;

		// Checkerboard pattern
		bool check = (int(floor(outHit.Position.x / 100.0f)) + int(floor(outHit.Position.z / 100.0f))) % 2 == 0;
		outHit.Color = check ? fvec3(0.9f) : fvec3(0.5f);

		return true;
	}
	return false;
}

bool GeometryPlane::IntersectsAny(const Ray& ray) const
{
	float distance;
	if (glm::intersectRayPlane(ray.position, ray.direction, origin, normal, distance))
	{
		return distance > 0;
	}
	return false;
}
