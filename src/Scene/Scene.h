#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Ray.h"

using namespace std;
using namespace glm;

class Geometry;

struct PointLight
{
	fvec3 Position;
	fvec3 Color;
	float Intensity;

	PointLight(fvec3 position, fvec3 color, float intensity)
		: Position(position), Color(color), Intensity(intensity)
	{ }
};

class Scene
{
public:
	// Directional lights

	// Ambient light
	fvec3 AmbientLightColor = { 1, 1, 1 };
	float AmbientIntensity = 1.f;

private:
	vector<Geometry*> _geometry;
	vector<PointLight*> _lights;

public:
	Scene();
	~Scene();

	void AddGeometry(Geometry* geometry);
	void AddLight(PointLight* light);
	const vector<Geometry*>& GetGeometryCollection() const;
	const vector<PointLight*>& GetLightCollection() const;
	bool Intersects(const Ray& ray, Hit& outHit) const;
	bool IntersectsAny(const Ray& ray) const;
};

