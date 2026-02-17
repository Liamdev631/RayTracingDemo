#pragma once
#include "Sphere.h"
#include <vector>

using namespace std;

/**
 * @brief Group of geometry with bounding-sphere culling for early exit tests.
 */
class Group : public Sphere
{
private:
	vector<Geometry const*> _geometry;

public:
	/**
	 * @brief Creates a group with a bounding sphere.
	 * @param center Bounding sphere center.
	 * @param radius Bounding sphere radius.
	 */
	Group(const fvec3& center, const float& radius);
	/**
	 * @brief Destroys the group.
	 */
	~Group();

	/**
	 * @brief Tests ray intersection across grouped geometry.
	 * @param scene Scene context for dependent geometry.
	 * @param ray Ray to test.
	 * @param outHit Populated with the closest hit on success.
	 * @return True if any geometry is intersected.
	 */
	bool Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const override;
	/**
	 * @brief Tests whether any intersection occurs.
	 * @param ray Ray to test.
	 * @return True if any geometry is intersected.
	 */
	bool IntersectsAny(const Ray& ray) const override;
	/**
	 * @brief Adds geometry to the group.
	 * @param geometry Geometry to reference.
	 */
	void AddGeometry(const Geometry& geometry);
};
