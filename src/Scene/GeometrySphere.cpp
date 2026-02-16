#include "GeometrySphere.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

using namespace glm;

GeometrySphere::GeometrySphere(const fvec3& center, const float& radius) noexcept
	: center(center), radius(radius)
{

}

GeometrySphere::~GeometrySphere() noexcept
{
	
}

bool GeometrySphere::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	bool hit = glm::intersectRaySphere<fvec3>(ray.position, ray.direction, center, radius, outHit.Position, outHit.Normal);
	if (!hit)
		return false;
	outHit.Distance = length(outHit.Position - ray.position);

	// Just return base color (Albedo)
	// SceneRenderer handles lighting
	outHit.Color = { 0.8f, 0.8f, 0.8f }; 

	return true;
}

bool GeometrySphere::IntersectsAny(const Ray& ray) const
{
	Hit dummyHit;
	return glm::intersectRaySphere<fvec3>(ray.position, ray.direction, center, radius, dummyHit.Position, dummyHit.Normal);
}
