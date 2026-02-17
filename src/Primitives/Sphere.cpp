#include "Sphere.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

using namespace glm;

Sphere::Sphere(const fvec3& center, const float& radius) noexcept
	: center(center), radius(radius)
{

}

Sphere::~Sphere() noexcept
{
	
}

bool Sphere::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	bool hit = glm::intersectRaySphere<fvec3>(ray.position, ray.direction, center, radius, outHit.Position, outHit.Normal);
	if (!hit)
		return false;
	outHit.Distance = length(outHit.Position - ray.position);

	// Just return base color (Albedo)
	// SceneRenderer handles lighting
	outHit.Color = { 0.8f, 0.8f, 0.8f }; 
	outHit.HitGeometry = this;

	return true;
}

bool Sphere::IntersectsAny(const Ray& ray) const
{
	Hit dummyHit;
	return glm::intersectRaySphere<fvec3>(ray.position, ray.direction, center, radius, dummyHit.Position, dummyHit.Normal);
}

void Sphere::GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const
{
	outAlbedo = { 0.8f, 0.8f, 0.8f };
	outRoughness = roughness;
	outMetallic = metallic;
	outAlpha = 0.0f; // Opaque
	outNormal = glm::normalize(p - center);
}
