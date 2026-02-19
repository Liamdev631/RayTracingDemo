#pragma once
#include "../Scene/Geometry/Geometry.h"

/**
 * @brief Sphere primitive with bounding data and PBR parameters.
 */
class Sphere : public Geometry
{
public:
	/** @brief Center position in world space. */
	fvec3 center;
	/** @brief Radius in world units. */
	float radius;
	/** @brief Default roughness for PBR shading. */
	float roughness = 0.5f;
	/** @brief Default metallic value for PBR shading. */
	float metallic = 0.0f;
	/** @brief Default alpha value for PBR shading. */
	float alpha = 1.0f;

public:
	/**
	 * @brief Creates a sphere by center and radius.
	 * @param center Center position in world space.
	 * @param radius Radius in world units.
	 */
	Sphere(const fvec3& center, const float& radius) noexcept;
	/**
	 * @brief Destroys the sphere.
	 */
	~Sphere() noexcept;

	/**
	 * @brief Tests ray intersection and outputs hit data.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the closest hit on success.
	 * @return True if an intersection occurs.
	 */
	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	/**
	 * @brief Tests whether any intersection occurs.
	 * @param ray Ray to test.
	 * @return True if any intersection occurs.
	 */
	bool IntersectsAny(const Ray& ray) const override;
	/**
	 * @brief Retrieves PBR parameters at a given position.
	 * @param p Position on the surface.
	 * @param outAlbedo Albedo color at the position.
	 * @param outRoughness Roughness scalar at the position.
	 * @param outMetallic Metallic scalar at the position.
	 * @param outAlpha Alpha scalar at the position.
	 * @param outNormal Shading normal at the position.
	 */
	virtual void GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const override;
};
