#include "Triangle.h"
#include <cmath>
#include <iostream>

static fvec3 SampleTexture(const sf::Image* texture, fvec2 uv)
{
	if (!texture) return { 1, 1, 1 };
	int w = texture->getSize().x;
	int h = texture->getSize().y;

	// Repeat wrap
	uv.x = uv.x - std::floor(uv.x);
	uv.y = uv.y - std::floor(uv.y);
    
    // Flip Y for texture coordinates if needed (standard is bottom-up, SFML is top-down)
    uv.y = 1.0f - uv.y; 

	int x = (int)(uv.x * w);
	int y = (int)(uv.y * h);

	// Clamp
	if (x < 0) x = 0; if (x >= w) x = w - 1;
	if (y < 0) y = 0; if (y >= h) y = h - 1;

	sf::Color c = texture->getPixel(x, y);
	return Color_SF_To_GLM(c);
}

// Möller–Trumbore intersection algorithm
bool Triangle::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
{
	const float EPSILON = 1e-6f;
	fvec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = v1 - v0;
	edge2 = v2 - v0;
	h = glm::cross(ray.direction, edge2);
	a = glm::dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return false; // This ray is parallel to this triangle.

	f = 1.0f / a;
	s = ray.position - v0;
	u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f)
		return false;

	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * glm::dot(edge2, q);

	if (t > EPSILON) // ray intersection
	{
		outHit.Distance = t;
		outHit.Position = ray.position + ray.direction * t;
		outHit.Normal = normal;
		// Ensure normal points towards the ray origin
		if (glm::dot(outHit.Normal, ray.direction) > 0)
			outHit.Normal = -outHit.Normal;
		
		outHit.HitGeometry = this;
		
        // Base color for debug/legacy
		outHit.Color = material->albedoColor;

		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}

bool Triangle::IntersectsAny(const Ray& ray) const
{
	const float EPSILON = 1e-6f;
	fvec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = v1 - v0;
	edge2 = v2 - v0;
	h = glm::cross(ray.direction, edge2);
	a = glm::dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return false;

	f = 1.0f / a;
	s = ray.position - v0;
	u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f)
		return false;

	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = f * glm::dot(edge2, q);

	if (t > EPSILON)
		return true;
	return false;
}

void Triangle::GetPBR(const fvec3& p, fvec3& outAlbedo, float& outRoughness, float& outMetallic, float& outAlpha, fvec3& outNormal) const
{
    // Compute Barycentrics
    fvec3 v0v1 = v1 - v0;
    fvec3 v0v2 = v2 - v0;
    fvec3 v0p = p - v0;

    float d00 = glm::dot(v0v1, v0v1);
    float d01 = glm::dot(v0v1, v0v2);
    float d11 = glm::dot(v0v2, v0v2);
    float d20 = glm::dot(v0p, v0v1);
    float d21 = glm::dot(v0p, v0v2);

    float denom = d00 * d11 - d01 * d01;
    
    float v_coord, w_coord;
    
    if (std::abs(denom) < 1e-6f) {
        v_coord = 0.0f; w_coord = 0.0f;
    } else {
        v_coord = (d11 * d20 - d01 * d21) / denom;
        w_coord = (d00 * d21 - d01 * d20) / denom;
    }
    float u_coord = 1.0f - v_coord - w_coord;

    // Interpolate UVs
    // P = u_coord*v0 + v_coord*v1 + w_coord*v2
    fvec2 texCoord = u_coord * uv0 + v_coord * uv1 + w_coord * uv2;

    // Albedo
    if (material->albedoMap)
        outAlbedo = SampleTexture(material->albedoMap, texCoord) * material->albedoColor;
    else
        outAlbedo = material->albedoColor;

    // Roughness
    if (material->roughnessMap)
        outRoughness = SampleTexture(material->roughnessMap, texCoord).r;
    else if (material->glossMap)
        outRoughness = 1.0f - SampleTexture(material->glossMap, texCoord).r;
    else
        outRoughness = material->roughnessVal;

    // Metallic
    if (material->metallicMap)
        outMetallic = SampleTexture(material->metallicMap, texCoord).r;
    else
        outMetallic = material->metallicVal;

    // Alpha
    if (material->alphaMap)
        outAlpha = SampleTexture(material->alphaMap, texCoord).r;
    else
        outAlpha = 0.0f; // Default to opaque if no map

    // Normal
    if (material->normalMap)
    {
        fvec3 mapN = SampleTexture(material->normalMap, texCoord);
        // Map [0,1] to [-1,1]
        mapN = mapN * 2.0f - 1.0f;
        
        fvec3 N = normal;
        fvec3 T = tangent;
        // Gram-Schmidt re-orthogonalize T with respect to N
        T = glm::normalize(T - N * glm::dot(N, T));
        fvec3 B = glm::cross(N, T);
        
        // TBN Matrix
        glm::mat3 TBN(T, B, N);
        outNormal = glm::normalize(TBN * mapN);
    }
    else
    {
        outNormal = normal;
    }
}
