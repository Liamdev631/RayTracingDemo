#include "CheckerCircle.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

CheckerCircle::CheckerCircle(const fvec3& origin, const fvec3& normal, float radius)
	: Sphere(origin, radius), normal(normal)
{

}

bool CheckerCircle::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	if (!Sphere::IntersectsAny(ray))
		return false;

	float distance;
	if (glm::intersectRayPlane(ray.position, ray.direction, center, normal, distance))
	{
		if (distance < 0) return false;
		
		fvec3 hitPos = ray.position + ray.direction * distance;
		
		// Check radius (distance from origin on the plane)
		// Assuming origin is the center of the circle
		if (glm::distance(hitPos, center) > radius)
		{
			return false;
		}

		outHit.Distance = distance;
		outHit.Position = hitPos;
		outHit.Normal = normal;

		// Checkerboard pattern
		bool check = (int(floor(outHit.Position.x / 100.0f)) + int(floor(outHit.Position.z / 100.0f))) % 2 == 0;
		outHit.Color = check ? fvec3(0.9f) : fvec3(0.5f);
		outHit.HitGeometry = this;

		return true;
	}
	return false;
}

bool CheckerCircle::IntersectsAny(const Ray& ray) const
{
	if (!Sphere::IntersectsAny(ray))
		return false;

	float distance;
	if (glm::intersectRayPlane(ray.position, ray.direction, center, normal, distance))
	{
		if (distance > 0)
		{
			fvec3 hitPos = ray.position + ray.direction * distance;
			return glm::distance(hitPos, center) <= radius;
		}
	}
	return false;
}

void CheckerCircle::GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const
{
	bool check = (int(floor(p.x / 100.0f)) + int(floor(p.z / 100.0f))) % 2 == 0;
	outAlbedo = check ? fvec3(0.9f) : fvec3(0.5f);
	// White tiles (check) get the base roughness (reflective), Dark tiles get 1.0 (matte)
	outRoughness = check ? roughness : 0.9f; 
	outMetallic = metallic;
	outAlpha = 0.0f; // Opaque
	outNormal = normal;
}
