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

	/* Calculate color
	fvec3 baseColor = Color_SF_To_GLM(sf::Color::Green);
	fvec3 diffuseColor;
	fvec3 ambientColor = scene->LightColor * scene->AmbientIntensity;
	float diffuseFactor = dot(normalize(outHit.Normal), -scene->LightDirection);
	if (diffuseFactor > 0)
		diffuseColor = scene->LightColor * scene->LightIntensity * diffuseFactor;
	else
		diffuseColor = { 0, 0, 0 };
	outHit.Color = baseColor * (diffuseColor + ambientColor);/*

	return true;*/

	fvec3 baseColor = { 0.8, 0.8f, 0.8 };
	fvec3 blendColor = { 0, 0, 0 };
	blendColor += scene->AmbientLightColor * scene->AmbientIntensity;

	// Cast rays from this point to every light source
	Ray lightRay;
	lightRay.position = outHit.Position + outHit.Normal * 0.01f;
	for (const auto& light : scene->GetLightCollection())
	{
		lightRay.direction = -normalize(light->Position - outHit.Position);
		if (!scene->IntersectsAny(lightRay))
		{
			float distance = length2(light->Position - outHit.Position);
			float diffuseFactor = dot(normalize(outHit.Normal), lightRay.direction);
			if (diffuseFactor > 0)
				blendColor += light->Color * light->Intensity * diffuseFactor / distance*distance;
		}
	}


	outHit.Color = baseColor * blendColor;

	return true;
}

bool GeometrySphere::IntersectsAny(const Ray& ray) const
{
	Hit dummyHit;
	return glm::intersectRaySphere<fvec3>(ray.position, ray.direction, center, radius, dummyHit.Position, dummyHit.Normal);
}
