#include "Cube.h"
#include "../Scene/Geometry/Triangle.h"

Cube::Cube(fvec3 center, float size, glm::mat4 transform, std::shared_ptr<PBRMaterial> material)
    : Mesh(GenerateTriangles(center, size, transform), material)
{
}

std::vector<Triangle> Cube::GenerateTriangles(fvec3 center, float size, glm::mat4 transform)
{
    std::vector<Triangle> tris;
    tris.reserve(12);

    float h = size * 0.5f;
    
    // Local vertices relative to center (before transform)
    // Front face (Z+)
    fvec3 v0 = fvec3(-h, -h,  h);
    fvec3 v1 = fvec3( h, -h,  h);
    fvec3 v2 = fvec3( h,  h,  h);
    fvec3 v3 = fvec3(-h,  h,  h);
    
    // Back face (Z-)
    fvec3 v4 = fvec3(-h, -h, -h);
    fvec3 v5 = fvec3( h, -h, -h);
    fvec3 v6 = fvec3( h,  h, -h);
    fvec3 v7 = fvec3(-h,  h, -h);
    
    // Apply transform and center offset
    // Wait, center is usually the position. If transform is Identity, cube is at center.
    // So vertices should be (local + center).
    // Or transform is applied to (local + center)?
    // User said: "constructs a cube from a center position, a size ... and a world space transform matrix."
    // If transform is meant to position the cube, center might be (0,0,0) or relative offset.
    // Let's assume center is the origin of the cube, and transform is applied ON TOP of that.
    // So P_world = Transform * (P_local + Center).
    
    auto transformPt = [&](fvec3 p) {
        return fvec3(transform * glm::vec4(p + center, 1.0f));
    };
    
    fvec3 t0 = transformPt(v0);
    fvec3 t1 = transformPt(v1);
    fvec3 t2 = transformPt(v2);
    fvec3 t3 = transformPt(v3);
    fvec3 t4 = transformPt(v4);
    fvec3 t5 = transformPt(v5);
    fvec3 t6 = transformPt(v6);
    fvec3 t7 = transformPt(v7);

    // Helper to add quad (2 triangles)
    // a=BL, b=BR, c=TR, d=TL
    auto addQuad = [&](fvec3 a, fvec3 b, fvec3 c, fvec3 d) {
        // Tri 1: a, b, c
        Triangle tri1(a, b, c, fvec3(1.0f));
        tri1.SetUVs(fvec2(0, 0), fvec2(1, 0), fvec2(1, 1));
        
        // Tri 2: a, c, d
        Triangle tri2(a, c, d, fvec3(1.0f));
        tri2.SetUVs(fvec2(0, 0), fvec2(1, 1), fvec2(0, 1));
        
        tris.push_back(tri1);
        tris.push_back(tri2);
    };

    // Front (0, 1, 2, 3)
    addQuad(t0, t1, t2, t3);
    
    // Back (5, 4, 7, 6)
    addQuad(t5, t4, t7, t6);
    
    // Left (4, 0, 3, 7)
    addQuad(t4, t0, t3, t7);
    
    // Right (1, 5, 6, 2)
    addQuad(t1, t5, t6, t2);
    
    // Top (3, 2, 6, 7)
    addQuad(t3, t2, t6, t7);
    
    // Bottom (4, 5, 1, 0)
    addQuad(t4, t5, t1, t0);

    return tris;
}
