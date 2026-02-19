#pragma once
#include "Sphere.h"
#include "../Scene/Geometry/Triangle.h"
#include <vector>
#include "../Material.h"
#include <memory>

/**
 * @brief Triangle mesh with a computed bounding sphere and optional material.
 */
class Mesh : public Sphere
{
public:
	/** @brief Triangle list in world space. */
	std::vector<Triangle> triangles;
    /** @brief Shared material applied to all triangles. */
    std::shared_ptr<PBRMaterial> material;

public:
	/**
	 * @brief Creates a mesh from triangles and optional shared material.
	 * @param triangles Triangle list in world space.
	 * @param material Optional shared material.
	 */
	Mesh(const std::vector<Triangle>& triangles, std::shared_ptr<PBRMaterial> material = nullptr);
	/** @brief Destroys the mesh. */
	~Mesh() = default;

	/**
	 * @brief Tests ray intersection across mesh triangles.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the closest hit on success.
	 * @return True if any triangle is intersected.
	 */
	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	/**
	 * @brief Tests whether any triangle is intersected.
	 * @param ray Ray to test.
	 * @return True if any triangle is intersected.
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

private:
    /** @brief Computes the bounding sphere from triangle vertices. */
    void CalculateBoundingSphere();
};
