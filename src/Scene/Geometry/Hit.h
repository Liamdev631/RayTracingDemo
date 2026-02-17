#pragma once
#include <glm/glm.hpp>

using namespace glm;

class Geometry;

/**
 * @brief Stores ray hit details for shading and PBR evaluation.
 */
struct Hit
{
	/** @brief World-space position of the intersection. */
	fvec3 Position;
	/** @brief Surface normal at the intersection. */
	fvec3 Normal;
	/** @brief Ray distance to the intersection. */
	float Distance;
	/** @brief Base albedo color at the intersection. */
	fvec3 Color;
	/** @brief Surface roughness used for PBR shading. */
	float Roughness = 0.5f;
	/** @brief Surface metallic value used for PBR shading. */
	float Metallic = 0.0f;
	/** @brief Surface transmission/alpha value (0 = opaque, 1 = transparent). */
	float Alpha = 0.0f;
	/** @brief Geometry that produced the hit. */
	const Geometry* HitGeometry = nullptr;

	/**
	 * @brief Assigns hit data from another instance.
	 * @param other Source hit to copy from.
	 * @return Reference to this instance.
	 */
	Hit& operator=(const Hit& other)
	{
		Position = other.Position;
		Normal = other.Normal;
		Distance = other.Distance;
		Color = other.Color;
		Roughness = other.Roughness;
		Metallic = other.Metallic;
		Alpha = other.Alpha;
		HitGeometry = other.HitGeometry;
		return *this;
	}
};
