#pragma once
#include "Mesh.h"
#include <glm/glm.hpp>

/**
 * @brief A rectangular plane (quad) generated as a triangle mesh.
 */
class Plane : public Mesh
{
public:
    /**
     * @brief Creates a plane mesh from a center, size, and transform.
     * @param center Plane center in world space before transform.
     * @param size Plane dimensions (width, height) in world units.
     * @param transform World transform applied to plane vertices.
     * @param material Shared material for the plane.
     * @param uvScale Scale factor applied to UV coordinates (default: 1.0).
     */
    Plane(fvec3 center, fvec2 size, glm::mat4 transform, std::shared_ptr<PBRMaterial> material, fvec2 uvScale = fvec2(1.0f));
    
private:
    /**
     * @brief Generates plane triangles (quad) with UVs.
     * @param center Plane center in world space before transform.
     * @param size Plane dimensions (width, height) in world units.
     * @param transform World transform applied to plane vertices.
     * @param uvScale Scale factor applied to UV coordinates.
     * @return Triangle list for the plane mesh.
     */
    static std::vector<Triangle> GenerateTriangles(fvec3 center, fvec2 size, glm::mat4 transform, fvec2 uvScale);
};
