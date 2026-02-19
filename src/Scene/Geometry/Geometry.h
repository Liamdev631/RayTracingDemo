#pragma once
#include "../Ray.h"
#include "../Scene.h"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Base interface for all renderable geometry.
 *
 * Implementations must provide intersection tests and PBR sampling data
 * used by the renderer for shading and reflections.
 */
class Geometry
{
public:
    /** @brief Name of the object for identification and animation. */
    std::string Name;

	/**
	 * @brief Tests ray intersection and outputs hit data.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the closest hit on success.
	 * @return True if an intersection occurs.
	 */
	virtual bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const = 0;
	/**
	 * @brief Tests whether any intersection occurs.
	 * @param ray Ray to test.
	 * @return True if any intersection occurs.
	 */
	virtual bool IntersectsAny(const Ray& ray) const = 0;
	/**
	 * @brief Retrieves PBR parameters at a given position.
	 * @param p Position on the surface.
	 * @param outAlbedo Albedo color at the position.
	 * @param outRoughness Roughness scalar at the position.
	 * @param outMetallic Metallic scalar at the position.
	 * @param outAlpha Alpha scalar at the position.
	 * @param outNormal Shading normal at the position.
	 */
	virtual void GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const = 0;
};

/**
 * @brief Converts GLM linear color to SFML color.
 * @param color Linear RGB in [0,1] range.
 * @return SFML color in 8-bit SRGB space.
 */
inline sf::Color Color_GLM_To_SF(fvec3 color)
{
	color = glm::clamp(color, { 0, 0, 0 }, { 1, 1, 1 });
	return sf::Color(sf::Uint8(color.r * 255.f), sf::Uint8(color.g * 255.f), sf::Uint8(color.b * 255.f));
}

/**
 * @brief Converts SFML color to GLM linear color.
 * @param c 8-bit SRGB color.
 * @return Linear RGB in [0,1] range.
 */
inline fvec3 Color_SF_To_GLM(sf::Color c)
{
	return { float(c.r) / 255.f, float(c.g) / 255.f, float(c.b) / 255.f };
}
