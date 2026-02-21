#pragma once
#include "Sphere.h"

/**
 * @brief Circular checkerboard plane with PBR parameters.
 */
class CheckerCircle : public Sphere
{
public:
	/** @brief Plane normal in world space. */
	fvec3 normal;

	/** @brief Tile size for checkerboard pattern. */
	float TileSize = 50.0f;

public:
	/**
	 * @brief Creates a checkerboard circle on a circular plane.
	 * @param origin Center of the circle in world space.
	 * @param normal Plane normal in world space.
	 * @param radius Circle radius in world units.
	 */
	CheckerCircle(const fvec3& origin, const fvec3& normal, float radius = 250.0f);
	/** @brief Destroys the checkerboard circle. */
	~CheckerCircle() = default;

	/**
	 * @brief Tests ray intersection with the checker circle.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the hit on success.
	 * @return True if the circle is intersected.
	 */
	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	/**
	 * @brief Tests whether any intersection occurs.
	 * @param ray Ray to test.
	 * @return True if the circle is intersected.
	 */
	bool IntersectsAny(const Ray& ray) const override;
	/**
	 * @brief Retrieves checkerboard PBR parameters at a given position.
	 * @param p Position on the surface.
	 * @param outAlbedo Albedo color at the position.
	 * @param outRoughness Roughness scalar at the position.
	 * @param outMetallic Metallic scalar at the position.
	 * @param outAlpha Alpha scalar at the position.
	 * @param outNormal Shading normal at the position.
	 */
	virtual void GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const override;
};
