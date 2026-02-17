#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>

Mesh::Mesh(const std::vector<Triangle>& triangles, std::shared_ptr<PBRMaterial> material)
	: Sphere(fvec3(0), 0), triangles(triangles), material(material)
{
    if (material)
    {
        for (auto& tri : this->triangles)
        {
            tri.SetMaterial(material);
        }
    }
    CalculateBoundingSphere();
}

void Mesh::CalculateBoundingSphere()
{
    if (triangles.empty())
    {
        center = fvec3(0.0f);
        radius = 0.0f;
        return;
    }

    // Simple bounding sphere: average center, max distance
    fvec3 minPt(std::numeric_limits<float>::max());
    fvec3 maxPt(std::numeric_limits<float>::lowest());

    for (const auto& tri : triangles)
    {
        minPt = glm::min(minPt, tri.v0);
        minPt = glm::min(minPt, tri.v1);
        minPt = glm::min(minPt, tri.v2);
        maxPt = glm::max(maxPt, tri.v0);
        maxPt = glm::max(maxPt, tri.v1);
        maxPt = glm::max(maxPt, tri.v2);
    }

    center = (minPt + maxPt) * 0.5f;
    radius = glm::distance(maxPt, center);
}

bool Mesh::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
    // Bounding sphere check
    if (!Sphere::IntersectsAny(ray))
    {
        return false;
    }

    bool hit = false;
    float closestDist = std::numeric_limits<float>::max();

    for (const auto& tri : triangles)
    {
        Hit triHit;
        if (tri.Intersects(scene, ray, triHit))
        {
            if (triHit.Distance < closestDist)
            {
                closestDist = triHit.Distance;
                outHit = triHit;
                hit = true;
            }
        }
    }

    return hit;
}

bool Mesh::IntersectsAny(const Ray& ray) const
{
    // Bounding sphere check
    if (!Sphere::IntersectsAny(ray))
    {
        return false;
    }

    for (const auto& tri : triangles)
    {
        if (tri.IntersectsAny(ray))
        {
            return true;
        }
    }
    return false;
}

void Mesh::GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, fvec3& outNormal) const
{
    // Should not be called directly if HitGeometry is set correctly to triangles
    // But if it is, we return a fallback.
    // The issue is that the renderer might call this on the mesh if HitGeometry was set to 'this'.
    // But Intersects() sets it to 'triHit' (which points to the triangle).
    // So this should be unreachable unless Intersects logic changes.
    outAlbedo = { 1.0f, 0.0f, 1.0f }; // Error pink
    outRoughness = 0.5f;
    outMetallic = 0.0f;
    outNormal = fvec3(0, 1, 0);
}
