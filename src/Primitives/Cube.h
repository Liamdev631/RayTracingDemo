#pragma once
#include "Mesh.h"
#include <glm/glm.hpp>

/**
 * @brief Axis-aligned cube generated as a triangle mesh.
 */
class Cube : public Mesh
{
public:
    /**
     * @brief Creates a cube mesh from a center, size, and transform.
     * @param center Cube center in world space before transform.
     * @param size Edge length in world units.
     * @param transform World transform applied to cube vertices.
     * @param material Shared material for all faces.
     */
    Cube(fvec3 center, float size, glm::mat4 transform, std::shared_ptr<PBRMaterial> material);
    
private:
    /**
     * @brief Generates cube triangles with UVs for each face.
     * @param center Cube center in world space before transform.
     * @param size Edge length in world units.
     * @param transform World transform applied to cube vertices.
     * @return Triangle list for the cube mesh.
     */
    static std::vector<Triangle> GenerateTriangles(fvec3 center, float size, glm::mat4 transform);
};
