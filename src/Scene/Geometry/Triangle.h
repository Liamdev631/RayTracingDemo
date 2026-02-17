#pragma once
#include "Geometry.h"
#include "../Material.h"
#include <memory>

/**
 * @brief Triangle primitive with UVs, tangent space, and PBR material data.
 */
class Triangle : public Geometry
{
public:
	fvec3 v0, v1, v2;
	fvec2 uv0, uv1, uv2;
	fvec3 normal;
	fvec3 tangent;
	
	std::shared_ptr<PBRMaterial> material;

	/**
	 * @brief Creates a triangle with vertex color as the default albedo.
	 * @param _v0 First vertex position.
	 * @param _v1 Second vertex position.
	 * @param _v2 Third vertex position.
	 * @param _color Default albedo color.
	 */
	Triangle(fvec3 _v0, fvec3 _v1, fvec3 _v2, fvec3 _color)
		: v0(_v0), v1(_v1), v2(_v2), uv0(0.0f), uv1(0.0f), uv2(0.0f), tangent(0.0f)
	{
		// Default material with vertex color as albedo
		material = std::make_shared<PBRMaterial>();
		material->albedoColor = _color;
		// Compute normal using cross product of two edges
		fvec3 edge1 = v1 - v0;
		fvec3 edge2 = v2 - v0;
		normal = glm::normalize(glm::cross(edge1, edge2));
	}
	
	/**
	 * @brief Assigns UV coordinates and recalculates the tangent.
	 * @param _uv0 UV for vertex v0.
	 * @param _uv1 UV for vertex v1.
	 * @param _uv2 UV for vertex v2.
	 */
	void SetUVs(fvec2 _uv0, fvec2 _uv1, fvec2 _uv2)
	{
		uv0 = _uv0;
		uv1 = _uv1;
		uv2 = _uv2;
		CalculateTangent();
	}

	/**
	 * @brief Assigns the material to the triangle.
	 * @param mat Material instance used for shading.
	 */
	void SetMaterial(std::shared_ptr<PBRMaterial> mat)
	{
		material = mat;
	}

	/**
	 * @brief Computes the tangent vector for normal mapping.
	 */
	void CalculateTangent()
	{
		fvec3 edge1 = v1 - v0;
		fvec3 edge2 = v2 - v0;
		fvec2 deltaUV1 = uv1 - uv0;
		fvec2 deltaUV2 = uv2 - uv0;
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = glm::normalize(tangent);
	}

	/**
	 * @brief Tests ray intersection and outputs hit data.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the closest hit on success.
	 * @return True if an intersection occurs.
	 */
	virtual bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	/**
	 * @brief Tests whether any intersection occurs.
	 * @param ray Ray to test.
	 * @return True if any intersection occurs.
	 */
	virtual bool IntersectsAny(const Ray& ray) const override;
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
