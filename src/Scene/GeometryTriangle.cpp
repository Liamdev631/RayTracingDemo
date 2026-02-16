#include "GeometryTriangle.h"

// Möller–Trumbore intersection algorithm
bool GeometryTriangle::Intersects(const Scene* scene, const Ray& ray, Hit& outHit) const
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
		
		outHit.Color = color;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}

bool GeometryTriangle::IntersectsAny(const Ray& ray) const
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
