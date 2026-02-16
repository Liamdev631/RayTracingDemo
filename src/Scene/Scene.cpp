#include "Scene.h"
#include "Geometry.h"

Scene::Scene()
{
	_geometry = vector<Geometry*>();
}

Scene::~Scene()
{
	for (auto geo : _geometry)
		delete geo;
	_geometry.clear();

	for (auto light : _lights)
		delete light;
	_lights.clear();
}

void Scene::AddGeometry(Geometry* geometry)
{
	_geometry.push_back(geometry);
}

void Scene::AddLight(PointLight* light)
{
	_lights.push_back(light);
}

const vector<Geometry*>& Scene::GetGeometryCollection() const
{
	return _geometry;
}

const vector<PointLight*>& Scene::GetLightCollection() const
{
	return _lights;
}

bool Scene::Intersects(const Ray& ray, Hit& outHit) const
{
	outHit.Distance = std::numeric_limits<float>::max();
	Hit temp;
	bool ret = false;
	for (auto iter = _geometry.begin(); iter != _geometry.end(); iter++)
		if ((*iter)->Intersects(this, ray, temp))
			if (temp.Distance < outHit.Distance)
			{
				outHit = temp;
				ret = true;
			}
	return ret;
}

bool Scene::IntersectsAny(const Ray& ray) const
{
	bool ret = false;
	for (auto iter = _geometry.begin(); iter != _geometry.end(); iter++)
		if ((*iter)->IntersectsAny(ray))
			return true;
	return false;
}

