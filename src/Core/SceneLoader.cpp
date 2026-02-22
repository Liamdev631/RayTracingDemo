#include "SceneLoader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "../Primitives/Sphere.h"
#include "../Primitives/CheckerCircle.h"
#include "../Scene/Geometry/Triangle.h"
#include "../Primitives/Mesh.h"
#include "../Primitives/Cube.h"
#include "../Primitives/Plane.h"
#include "MeshLoader.h"
#include "../Scene/Animation/Animator.h"

using namespace std;
using json = nlohmann::json;

const sf::Image* SceneLoader::GetOrLoadTexture(Scene* scene, std::map<string, int>& cache, const string& path)
{
    if (cache.find(path) != cache.end())
    {
        return scene->GetTexture(cache[path]);
    }
    
    int idx = scene->AddTexture(path);
    if (idx != -1)
    {
        cache[path] = idx;
        return scene->GetTexture(idx);
    }
    return nullptr;
}

shared_ptr<PBRMaterial> SceneLoader::ParseMaterial(const json& j, Scene* scene, std::map<string, int>& cache)
{
    auto mat = make_shared<PBRMaterial>();
    mat->albedoColor = fvec3(1.0f); // Default white

    // Helper lambda to parse material properties from a JSON object
    auto parseProps = [&](const json& m) {
        // Albedo
        if (m.contains("albedo")) {
            if (m["albedo"].is_string()) {
                // Try to load as texture
                const sf::Image* img = nullptr;
                string path = m["albedo"];
                if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                else {
                    int idx = scene->AddTexture(path);
                    if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                }
                if (img) mat->albedoMap = img;
            } else if (m["albedo"].is_array()) {
                mat->albedoColor = { m["albedo"][0], m["albedo"][1], m["albedo"][2] };
            }
        }
        
        // Color (legacy/alias for albedo color)
        if (m.contains("color")) {
            mat->albedoColor = { m["color"][0], m["color"][1], m["color"][2] };
        }

        // Roughness
        if (m.contains("roughness")) {
            if (m["roughness"].is_string()) {
                    const sf::Image* img = nullptr;
                    string path = m["roughness"];
                    if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                    else {
                        int idx = scene->AddTexture(path);
                        if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                    }
                    if (img) mat->roughnessMap = img;
            }
            else mat->roughnessVal = m["roughness"];
        }

        // Metallic
        if (m.contains("metallic")) {
            if (m["metallic"].is_string()) {
                    const sf::Image* img = nullptr;
                    string path = m["metallic"];
                    if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                    else {
                        int idx = scene->AddTexture(path);
                        if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                    }
                    if (img) mat->metallicMap = img;
            }
            else mat->metallicVal = m["metallic"];
        }

        // Normal
        if (m.contains("normal") && m["normal"].is_string()) {
                const sf::Image* img = nullptr;
                string path = m["normal"];
                if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                else {
                    int idx = scene->AddTexture(path);
                    if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                }
                if (img) mat->normalMap = img;
        }

        // Alpha
        if (m.contains("alpha")) {
            if (m["alpha"].is_string()) {
                    const sf::Image* img = nullptr;
                    string path = m["alpha"];
                    if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                    else {
                        int idx = scene->AddTexture(path);
                        if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                    }
                    if (img) mat->alphaMap = img;
            }
            else mat->alphaVal = m["alpha"];
        }
    };

    // 1. Load Base Material
    if (j.contains("material"))
    {
        if (j["material"].is_string())
        {
            // Load from .mat file
            string matPath = j["material"];
            ifstream matFile("assets/Materials/" + matPath); // Try relative to assets/Materials first
            if (!matFile.is_open())
            {
                    matFile.open(matPath); // Try absolute or relative to CWD
            }
            
            if (matFile.is_open())
            {
                json matJson;
                matFile >> matJson;
                parseProps(matJson);
            }
            else
            {
                cerr << "Failed to load material file: " << matPath << endl;
            }
        }
        else if (j["material"].is_object())
        {
            // Inline material definition
            parseProps(j["material"]);
        }
    }

    // 2. Apply Overrides (Top-level properties on the object)
    // These overwrite whatever was loaded from the base material
    parseProps(j); 
    
    // Handle legacy "texture" property as albedo map override
    if (j.contains("texture")) {
        if (j["texture"].is_string()) {
                const sf::Image* img = nullptr;
                string path = j["texture"];
                if (cache.find(path) != cache.end()) img = scene->GetTexture(cache[path]);
                else {
                    int idx = scene->AddTexture(path);
                    if (idx != -1) { cache[path] = idx; img = scene->GetTexture(idx); }
                }
                if (img) mat->albedoMap = img;
        } else if (j["texture"].is_number_integer()) {
            int idx = j["texture"];
            mat->albedoMap = scene->GetTexture(idx);
        }
    }

    return mat;
}

shared_ptr<Scene> SceneLoader::LoadScene(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open scene file: " << filename << endl;
        return nullptr;
    }

    json j;
    file >> j;

    auto scene = make_shared<Scene>();
    std::map<string, int> textureCache;
    
    // Reset Animator Singleton
    Animator::Get().Reset();

    // Load Scene Settings (Camera, Animation, Sky Light, Ambient)
    bool hasDirectionalLight = false;
    if (j.contains("settings"))
    {
        auto& settings = j["settings"];
        if (settings.contains("camera"))
        {
            auto& cam = settings["camera"];
            if (cam.contains("position")) {
                scene->CameraPosition = { cam["position"][0], cam["position"][1], cam["position"][2] };
                scene->InitialCameraPosition = scene->CameraPosition;
            }
            if (cam.contains("target")) {
                scene->CameraTarget = { cam["target"][0], cam["target"][1], cam["target"][2] };
            }
            if (cam.contains("fov")) {
                scene->CameraFOV = cam["fov"].get<float>();
            }
            
            // Camera Animation
            if (cam.contains("orbit_angle") || cam.contains("altitude")) {
                auto& anim = Animator::Get().CameraAnim;
                anim.Enabled = true;
                anim.Target = scene->CameraTarget;
                anim.UseTarget = true;
                
                if (cam.contains("orbit_angle")) {
                    if (cam["orbit_angle"].is_array()) {
                        anim.OrbitStart = cam["orbit_angle"][0];
                        anim.OrbitEnd = cam["orbit_angle"][1];
                    } else {
                        anim.OrbitStart = cam["orbit_angle"];
                        anim.OrbitEnd = anim.OrbitStart;
                    }
                }
                
                if (cam.contains("altitude")) {
                    if (cam["altitude"].is_array()) {
                        anim.AltitudeStart = cam["altitude"][0];
                        anim.AltitudeEnd = cam["altitude"][1];
                    } else {
                        anim.AltitudeStart = cam["altitude"];
                        anim.AltitudeEnd = anim.AltitudeStart;
                    }
                }
                
                if (cam.contains("orbit_distance")) {
                    anim.Distance = cam["orbit_distance"];
                } else {
                    anim.Distance = glm::length(scene->CameraPosition - scene->CameraTarget);
                }
            }
        }
        
        if (settings.contains("animation"))
        {
            auto& anim = settings["animation"];
            // Legacy support if needed, but we rely on Animator now.
        }

        // Load Sky Light (formerly Directional Light)
        if (settings.contains("sky_light"))
        {
            auto& sky = settings["sky_light"];
            auto& sunAnim = Animator::Get().SunAnim;
            
            float orbit = 0.0f;
            float altitude = 45.0f;
            if (sky.contains("orbit_angle")) {
                if (sky["orbit_angle"].is_array()) {
                    sunAnim.OrbitStart = sky["orbit_angle"][0];
                    sunAnim.OrbitEnd = sky["orbit_angle"][1];
                    orbit = sunAnim.OrbitStart;
                    sunAnim.Enabled = true;
                } else {
                    orbit = sky["orbit_angle"];
                    sunAnim.OrbitStart = orbit;
                    sunAnim.OrbitEnd = orbit;
                }
            }
            
            if (sky.contains("altitude")) {
                if (sky["altitude"].is_array()) {
                    sunAnim.AltitudeStart = sky["altitude"][0];
                    sunAnim.AltitudeEnd = sky["altitude"][1];
                    altitude = sunAnim.AltitudeStart;
                    sunAnim.Enabled = true;
                } else {
                    altitude = sky["altitude"];
                    sunAnim.AltitudeStart = altitude;
                    sunAnim.AltitudeEnd = altitude;
                }
            }

            fvec3 color = {
                sky["color"][0].get<float>(),
                sky["color"][1].get<float>(),
                sky["color"][2].get<float>()
            };
            float intensity = sky["intensity"].get<float>();
            
            // Calculate initial direction
            // Y is Up. Orbit is around Y axis. Altitude is angle from XZ plane.
            float orbitRad = glm::radians(orbit);
            float altRad = glm::radians(altitude);
            
            fvec3 sunPos(
                std::cos(altRad) * std::sin(orbitRad),
                std::sin(altRad),
                std::cos(altRad) * std::cos(orbitRad)
            );
            
            fvec3 dir = -glm::normalize(sunPos);
            
            scene->SunLight = DirectionalLight(dir, color, intensity);
            scene->InitialSunIntensity = intensity;
            hasDirectionalLight = true;
        }

        // Load Ambient Light
        if (settings.contains("ambient"))
        {
            auto& ambient = settings["ambient"];
            if (ambient.contains("color"))
            {
                scene->AmbientLightColor = {
                    ambient["color"][0].get<float>(),
                    ambient["color"][1].get<float>(),
                    ambient["color"][2].get<float>()
                };
            }
            if (ambient.contains("intensity"))
            {
                scene->AmbientIntensity = ambient["intensity"].get<float>();
            }
        }
    }

    // Backward compatibility for ambient_light (if not in settings)
    if (j.contains("ambient_light") && scene->AmbientIntensity == 0.0f)
    {
        auto& ambient = j["ambient_light"];
        if (ambient.contains("color"))
        {
            scene->AmbientLightColor = {
                ambient["color"][0].get<float>(),
                ambient["color"][1].get<float>(),
                ambient["color"][2].get<float>()
            };
        }
        if (ambient.contains("intensity"))
        {
            scene->AmbientIntensity = ambient["intensity"].get<float>();
        }
    }

    // Load Lights
    if (j.contains("lights"))
    {
        for (const auto& lightData : j["lights"])
        {
            string type = lightData["type"];
            if (type == "point")
            {
                fvec3 pos = {
                    lightData["position"][0].get<float>(),
                    lightData["position"][1].get<float>(),
                    lightData["position"][2].get<float>()
                };
                fvec3 color = {
                    lightData["color"][0].get<float>(),
                    lightData["color"][1].get<float>(),
                    lightData["color"][2].get<float>()
                };
                float intensity = lightData["intensity"].get<float>();
                auto light = new PointLight(pos, color, intensity);
                if (lightData.contains("name")) light->Name = lightData["name"];
                scene->AddLight(light);
            }
        }
    }
    
    if (!hasDirectionalLight)
    {
        cerr << "Warning: No sky_light (formerly directional light) found in scene. Using default." << endl;
    }

    // Load Textures (Legacy List)
    if (j.contains("textures"))
    {
        auto& textures = j["textures"];
        if (textures.is_array())
        {
            for (auto& path : textures)
            {
                GetOrLoadTexture(scene.get(), textureCache, path);
            }
        }
        else if (textures.is_object())
        {
            for (auto& element : textures.items())
            {
                GetOrLoadTexture(scene.get(), textureCache, element.value());
            }
        }
    }

    // Load Geometry
    if (j.contains("objects"))
    {
        for (const auto& objData : j["objects"])
        {
            string type = objData["type"];
            if (type == "sphere")
            {
                fvec3 pos = {
                    objData["position"][0].get<float>(),
                    objData["position"][1].get<float>(),
                    objData["position"][2].get<float>()
                };
                float radius = objData["radius"].get<float>();
                auto sphere = new Sphere(pos, radius);
                if (objData.contains("roughness")) sphere->roughness = objData["roughness"].get<float>();
                if (objData.contains("metallic")) sphere->metallic = objData["metallic"].get<float>();
                if (objData.contains("alpha")) sphere->alpha = objData["alpha"].get<float>();
                if (objData.contains("name")) sphere->Name = objData["name"];
                scene->AddGeometry(sphere);
            }
            else if (type == "checker_circle")
            {
                fvec3 origin = {
                    objData["origin"][0].get<float>(),
                    objData["origin"][1].get<float>(),
                    objData["origin"][2].get<float>()
                };
                fvec3 normal = {
                    objData["normal"][0].get<float>(),
                    objData["normal"][1].get<float>(),
                    objData["normal"][2].get<float>()
                };
                float radius = 250.0f;
                if (objData.contains("radius"))
                    radius = objData["radius"].get<float>();
                    
                auto cc = new CheckerCircle(origin, normal, radius);
                if (objData.contains("roughness")) cc->roughness = objData["roughness"].get<float>();
                if (objData.contains("metallic")) cc->metallic = objData["metallic"].get<float>();
                if (objData.contains("alpha")) cc->alpha = objData["alpha"].get<float>();
                if (objData.contains("name")) cc->Name = objData["name"];
                scene->AddGeometry(cc);
            }
            else if (type == "plane")
            {
                fvec3 center = { 0, 0, 0 };
                if (objData.contains("center"))
                    center = { objData["center"][0], objData["center"][1], objData["center"][2] };
                else if (objData.contains("origin"))
                    center = { objData["origin"][0], objData["origin"][1], objData["origin"][2] };
                    
                fvec2 size = { 100.0f, 100.0f };
                if (objData.contains("size"))
                {
                    if (objData["size"].is_array())
                        size = { objData["size"][0], objData["size"][1] };
                    else if (objData["size"].is_number())
                    {
                        float s = objData["size"];
                        size = { s, s };
                    }
                }
                
                glm::mat4 transform(1.0f);
                if (objData.contains("transform"))
                {
                    auto& t = objData["transform"];
                    if (t.size() == 16)
                    {
                            for (int i = 0; i < 16; i++)
                            transform[i / 4][i % 4] = t[i];
                    }
                }
                
                fvec2 uvScale = { 1.0f, 1.0f };
                if (objData.contains("uv_scale"))
                {
                    if (objData["uv_scale"].is_array())
                    {
                        uvScale.x = objData["uv_scale"][0];
                        uvScale.y = objData["uv_scale"][1];
                    }
                    else if (objData["uv_scale"].is_number())
                    {
                        float s = objData["uv_scale"];
                        uvScale = { s, s };
                    }
                }

                auto mat = ParseMaterial(objData, scene.get(), textureCache);
                auto plane = new Plane(center, size, transform, mat, uvScale);
                if (objData.contains("name")) plane->Name = objData["name"];
                scene->AddGeometry(plane);
            }
            else if (type == "triangle")
            {
                fvec3 v0 = {
                    objData["v0"][0].get<float>(),
                    objData["v0"][1].get<float>(),
                    objData["v0"][2].get<float>()
                };
                fvec3 v1 = {
                    objData["v1"][0].get<float>(),
                    objData["v1"][1].get<float>(),
                    objData["v1"][2].get<float>()
                };
                fvec3 v2 = {
                    objData["v2"][0].get<float>(),
                    objData["v2"][1].get<float>(),
                    objData["v2"][2].get<float>()
                };
                
                auto mat = ParseMaterial(objData, scene.get(), textureCache);
                auto tri = new Triangle(v0, v1, v2, fvec3(1.0f));
                tri->SetMaterial(mat);
                if (objData.contains("name")) tri->Name = objData["name"];
                
                if (objData.contains("uvs") && objData["uvs"].is_array() && objData["uvs"].size() == 6)
                {
                        fvec2 uv0 = { objData["uvs"][0], objData["uvs"][1] };
                        fvec2 uv1 = { objData["uvs"][2], objData["uvs"][3] };
                        fvec2 uv2 = { objData["uvs"][4], objData["uvs"][5] };
                        tri->SetUVs(uv0, uv1, uv2);
                }

                scene->AddGeometry(tri);
            }
            else if (type == "mesh")
            {
                if (objData.contains("file"))
                {
                    string filePath = objData["file"];
                    auto loadedParts = MeshLoader::LoadMesh(filePath);
                    
                    // Apply transforms if present
                    glm::mat4 transform(1.0f);
                    bool hasTransform = false;

                    // Position
                    if (objData.contains("position"))
                    {
                        fvec3 pos = {
                            objData["position"][0].get<float>(),
                            objData["position"][1].get<float>(),
                            objData["position"][2].get<float>()
                        };
                        transform = glm::translate(transform, pos);
                        hasTransform = true;
                    }

                    // Rotation (Euler angles in degrees)
                    if (objData.contains("rotation"))
                    {
                        fvec3 rot = {
                            objData["rotation"][0].get<float>(),
                            objData["rotation"][1].get<float>(),
                            objData["rotation"][2].get<float>()
                        };
                        transform = glm::rotate(transform, glm::radians(rot.x), fvec3(1, 0, 0));
                        transform = glm::rotate(transform, glm::radians(rot.y), fvec3(0, 1, 0));
                        transform = glm::rotate(transform, glm::radians(rot.z), fvec3(0, 0, 1));
                        hasTransform = true;
                    }

                    // Scale
                    if (objData.contains("scale"))
                    {
                            fvec3 scale = { 1, 1, 1 };
                            if (objData["scale"].is_array())
                                scale = { objData["scale"][0], objData["scale"][1], objData["scale"][2] };
                            else if (objData["scale"].is_number())
                            {
                                float s = objData["scale"];
                                scale = { s, s, s };
                            }
                            transform = glm::scale(transform, scale);
                            hasTransform = true;
                    }
                    
                    fvec2 uvScale = { 1.0f, 1.0f };
                    if (objData.contains("uv_scale"))
                    {
                        if (objData["uv_scale"].is_array())
                        {
                            uvScale.x = objData["uv_scale"][0];
                            uvScale.y = objData["uv_scale"][1];
                        }
                        else if (objData["uv_scale"].is_number())
                        {
                            float s = objData["uv_scale"];
                            uvScale = { s, s };
                        }
                    }

                    // Apply transform to all parts
                    for (auto& part : loadedParts)
                    {
                        if (hasTransform)
                        {
                            for (auto& tri : part->triangles)
                            {
                                tri.v0 = fvec3(transform * fvec4(tri.v0, 1.0f));
                                tri.v1 = fvec3(transform * fvec4(tri.v1, 1.0f));
                                tri.v2 = fvec3(transform * fvec4(tri.v2, 1.0f));
                                // Normals should be transformed by inverse transpose, but for uniform scale/rotation, this is approx ok, or we need proper handling.
                                // For now, let's just recompute normals or assume they are close enough.
                                // Actually, if we rotate, normals MUST rotate.
                                // Correct way: transform normal by mat3(transpose(inverse(transform)))
                                glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));
                                tri.normal = glm::normalize(normalMat * tri.normal);
                            }
                            part->center = fvec3(transform * fvec4(part->center, 1.0f));
                            part->radius = 0; // Recompute? For now, leave it.
                        }
                        
                        // Apply material overrides if present
                        // Note: MeshLoader already assigns a material, but we can override it via Scene JSON
                        // If "material" is specified in the object block, it overrides ALL parts.
                        // If not, it uses what's in the file.
                        if (objData.contains("material") || objData.contains("texture") || objData.contains("roughness") || objData.contains("metallic"))
                        {
                            auto overrideMat = ParseMaterial(objData, scene.get(), textureCache);
                            part->material = overrideMat;
                            for (auto& tri : part->triangles)
                            {
                                tri.SetMaterial(overrideMat);
                            }
                        }
                        
                        if (objData.contains("name")) part->Name = objData["name"];
                            else if (part->Name.empty()) part->Name = "MeshPart";

                            scene->AddGeometry(part);
                    }
                }
            }
            else if (type == "cube")
            {
                fvec3 center = { 0, 0, 0 };
                if (objData.contains("center"))
                    center = { objData["center"][0], objData["center"][1], objData["center"][2] };
                
                float size = 50.0f;
                if (objData.contains("size")) size = objData["size"];

                glm::mat4 transform(1.0f);
                if (objData.contains("transform"))
                {
                    auto& t = objData["transform"];
                    if (t.size() == 16)
                    {
                            for (int i = 0; i < 16; i++)
                            transform[i / 4][i % 4] = t[i];
                    }
                }
                
                fvec2 uvScale = { 1.0f, 1.0f };
                if (objData.contains("uv_scale"))
                {
                    if (objData["uv_scale"].is_array())
                    {
                        uvScale.x = objData["uv_scale"][0];
                        uvScale.y = objData["uv_scale"][1];
                    }
                    else if (objData["uv_scale"].is_number())
                    {
                        float s = objData["uv_scale"];
                        uvScale = { s, s };
                    }
                }

                auto mat = ParseMaterial(objData, scene.get(), textureCache);
                auto cube = new Cube(center, size, transform, mat, uvScale);
                if (objData.contains("name")) cube->Name = objData["name"];
                scene->AddGeometry(cube);
            }
        }
    }

    // Load Animators (KeyframeTracks)
    if (j.contains("animators"))
    {
        for (const auto& animData : j["animators"])
        {
            KeyframeTrack anim;
            anim.TargetName = animData["target"];
            anim.Property = animData["property"];
            if (animData.contains("normalized")) anim.NormalizedTime = animData["normalized"];
            if (animData.contains("loop")) anim.Loop = animData["loop"];
            
            if (animData.contains("keyframes"))
            {
                for (const auto& kfData : animData["keyframes"])
                {
                    Keyframe kf;
                    kf.Time = kfData["time"];
                    if (kfData["value"].is_array())
                    {
                        kf.Value = {
                            kfData["value"][0].get<float>(),
                            kfData["value"][1].get<float>(),
                            kfData["value"][2].get<float>()
                        };
                    }
                    else if (kfData["value"].is_number())
                    {
                        float v = kfData["value"];
                        kf.Value = { v, 0, 0 }; // Scalar stored in x
                    }
                    anim.Keyframes.push_back(kf);
                }
                // Sort keyframes
                std::sort(anim.Keyframes.begin(), anim.Keyframes.end(), [](const Keyframe& a, const Keyframe& b){
                    return a.Time < b.Time;
                });
            }
            
            scene->KeyframeTracks.push_back(anim);
        }
    }

    return scene;
}