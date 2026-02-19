#include "Plane.h"
#include "../Scene/Geometry/Triangle.h"

Plane::Plane(fvec3 center, fvec2 size, glm::mat4 transform, std::shared_ptr<PBRMaterial> material, fvec2 uvScale)
    : Mesh(GenerateTriangles(center, size, transform, uvScale), material)
{
}

std::vector<Triangle> Plane::GenerateTriangles(fvec3 center, fvec2 size, glm::mat4 transform, fvec2 uvScale)
{
    std::vector<Triangle> tris;
    tris.reserve(2);

    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;
    
    // Local vertices relative to center (before transform)
    // Vertical Plane (XY plane), facing +Z
    // BL, BR, TR, TL
    fvec3 v0 = fvec3(-hw, -hh, 0.0f);
    fvec3 v1 = fvec3( hw, -hh, 0.0f);
    fvec3 v2 = fvec3( hw,  hh, 0.0f);
    fvec3 v3 = fvec3(-hw,  hh, 0.0f);
    
    // Apply transform and center offset
    auto transformPt = [&](fvec3 p) {
        return fvec3(transform * glm::vec4(p + center, 1.0f));
    };
    
    fvec3 t0 = transformPt(v0);
    fvec3 t1 = transformPt(v1);
    fvec3 t2 = transformPt(v2);
    fvec3 t3 = transformPt(v3);

    // Helper to add quad (2 triangles)
    // a=BL, b=BR, c=TR, d=TL
    auto addQuad = [&](fvec3 a, fvec3 b, fvec3 c, fvec3 d) {
        // Tri 1: a, b, c
        Triangle tri1(a, b, c, fvec3(1.0f));
        tri1.SetUVs(fvec2(0, 0) * uvScale, fvec2(1, 0) * uvScale, fvec2(1, 1) * uvScale);
        
        // Tri 2: a, c, d
        Triangle tri2(a, c, d, fvec3(1.0f));
        tri2.SetUVs(fvec2(0, 0) * uvScale, fvec2(1, 1) * uvScale, fvec2(0, 1) * uvScale);
        
        tris.push_back(tri1);
        tris.push_back(tri2);
    };

    // Add the quad (facing +Z)
    addQuad(t0, t1, t2, t3);

    return tris;
}
