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

int Scene::AddTexture(const string& filepath)
{
	auto img = make_shared<sf::Image>();
	if (img->loadFromFile(filepath))
	{
		_textures.push_back(img);
		printf("Loaded texture: %d from %s\n", (int)_textures.size() - 1, filepath.c_str());
		return (int)_textures.size() - 1;
	}
	else
	{
		printf("Failed to load texture from %s\n", filepath.c_str());
		return -1;
	}
}

const sf::Image* Scene::GetTexture(int index) const
{
	if (index >= 0 && index < _textures.size())
		return _textures[index].get();
	return nullptr;
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

void Scene::Update(float time, float duration)
{
    // Apply keyframe animations first (if any)
    for (auto& animator : Animators)
    {
        animator.Apply(this, time, duration);
    }

    // Apply linear sun interpolation
    if (duration > 0.0f)
    {
        float t = glm::clamp(time / duration, 0.0f, 1.0f);
        
        float currentOrbit = glm::mix(SunOrbitStart, SunOrbitEnd, t);
        float currentAltitude = glm::mix(SunAltitudeStart, SunAltitudeEnd, t);
        
        float orbitRad = glm::radians(currentOrbit);
        float altRad = glm::radians(currentAltitude);

        // Y is Up.
        // Orbit is rotation around Y axis (azimuth).
        // Altitude is angle from XZ plane.
        // x = cos(alt) * sin(orbit)
        // y = sin(alt)
        // z = cos(alt) * cos(orbit)
        fvec3 sunPos(
            std::cos(altRad) * std::sin(orbitRad),
            std::sin(altRad),
            std::cos(altRad) * std::cos(orbitRad)
        );
        
        // Direction is from Sun to Origin, so negative of Position (assuming Origin is 0,0,0)
        SunLight.Direction = -glm::normalize(sunPos);
        
        // Optional: Horizon dimming
        // If direction is pointing UP (y > 0), it's below horizon -> intensity 0
        // Wait, SunLight.Direction is from Sun to Origin.
        // So if Sun is at (0, 100, 0), direction is (0, -1, 0).
        // If Sun is below horizon (0, -100, 0), direction is (0, 1, 0).
        // So if direction.y > 0, Sun is below horizon.
        if (SunLight.Direction.y > 0)
        {
             SunLight.Intensity = 0.0f;
        }
        else
        {
             SunLight.Intensity = InitialSunIntensity;
        }
    }
}
