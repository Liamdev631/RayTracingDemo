#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "Scene.h"
#include "../Primitives/Sphere.h"
#include "../Primitives/CheckerCircle.h"
#include "../Scene/Geometry/Triangle.h"
#include "../Primitives/Mesh.h"
#include "../Primitives/Cube.h"
#include "../Primitives/Plane.h"
#include "Material.h"
#include "MeshLoader.h"
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using json = nlohmann::json;
using namespace std;

/**
 * @brief Loads scene data from JSON files into runtime scene objects.
 */
class SceneLoader
{
private:
    /**
     * @brief Loads a texture from the scene, caching it if not already loaded.
     * @param scene The scene to load the texture into.
     * @param cache A cache mapping texture paths to texture indices.
     * @param path The path to the texture file.
     * @return A pointer to the loaded texture, or nullptr if loading failed.
     */
    static const sf::Image* GetOrLoadTexture(Scene* scene, std::map<string, int>& cache, const string& path)
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

    static shared_ptr<PBRMaterial> ParseMaterial(const json& j, Scene* scene, std::map<string, int>& cache)
    {
        auto mat = make_shared<PBRMaterial>();
        
        // Default color (white)
        mat->albedoColor = fvec3(1.0f);

        // Check for simple texture (legacy or simple assignment)
        if (j.contains("texture"))
        {
            if (j["texture"].is_string())
            {
                mat->albedoMap = GetOrLoadTexture(scene, cache, j["texture"]);
            }
            else if (j["texture"].is_number_integer())
            {
                int idx = j["texture"];
                mat->albedoMap = scene->GetTexture(idx);
            }
        }
        
        // Check for color (legacy)
        if (j.contains("color"))
        {
             mat->albedoColor = {
                j["color"][0].get<float>(),
                j["color"][1].get<float>(),
                j["color"][2].get<float>()
            };
        }

        // Check for top-level PBR properties (if not inside material)
        if (j.contains("roughness"))
        {
             if (j["roughness"].is_number()) mat->roughnessVal = j["roughness"];
             else if (j["roughness"].is_string()) mat->roughnessMap = GetOrLoadTexture(scene, cache, j["roughness"]);
        }
        if (j.contains("metallic"))
        {
             if (j["metallic"].is_number()) mat->metallicVal = j["metallic"];
             else if (j["metallic"].is_string()) mat->metallicMap = GetOrLoadTexture(scene, cache, j["metallic"]);
        }

        // Advanced PBR material
        if (j.contains("material"))
        {
            auto& m = j["material"];
            
            // Albedo
            if (m.contains("albedo"))
            {
                if (m["albedo"].is_string())
                {
                    mat->albedoMap = GetOrLoadTexture(scene, cache, m["albedo"]);
                }
                else if (m["albedo"].is_array())
                    mat->albedoColor = { m["albedo"][0], m["albedo"][1], m["albedo"][2] };
            }
            
            // Color (inside material)
            if (m.contains("color"))
            {
                mat->albedoColor = {
                    m["color"][0].get<float>(),
                    m["color"][1].get<float>(),
                    m["color"][2].get<float>()
                };
            }
            
            // Roughness
            if (m.contains("roughness"))
            {
                if (m["roughness"].is_string())
                    mat->roughnessMap = GetOrLoadTexture(scene, cache, m["roughness"]);
                else
                    mat->roughnessVal = m["roughness"];
            }
            
            // Metallic
            if (m.contains("metallic"))
            {
                if (m["metallic"].is_string())
                    mat->metallicMap = GetOrLoadTexture(scene, cache, m["metallic"]);
                else
                    mat->metallicVal = m["metallic"];
            }
            
            // Normal
            if (m.contains("normal"))
            {
                 mat->normalMap = GetOrLoadTexture(scene, cache, m["normal"]);
            }
            
            // Gloss
            if (m.contains("gloss"))
            {
                 mat->glossMap = GetOrLoadTexture(scene, cache, m["gloss"]);
            }

            // Alpha
            if (m.contains("alpha"))
            {
                if (m["alpha"].is_string())
                    mat->alphaMap = GetOrLoadTexture(scene, cache, m["alpha"]);
                else if (m["alpha"].is_number())
                    mat->alphaVal = m["alpha"];
            }
        }
        
        return mat;
    }

public:
    /**
     * @brief Loads a scene from a JSON file on disk.
     * @param filename Path to the scene file.
     * @return Scene instance or nullptr on failure.
     */
    static shared_ptr<Scene> LoadScene(const string& filename)
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
                }
                
                if (settings.contains("animation"))
            {
                auto& anim = settings["animation"];
                if (anim.contains("camera_speed"))
                    scene->CameraRotationSpeed = anim["camera_speed"];
                if (anim.contains("sun_speed"))
                    scene->SunRotationSpeed = anim["sun_speed"];
            }

            // Load Sky Light (formerly Directional Light)
            if (settings.contains("sky_light"))
            {
                auto& sky = settings["sky_light"];
                
                float orbit = 0.0f;
                float altitude = 45.0f;
                if (sky.contains("orbit_angle")) {
                    if (sky["orbit_angle"].is_array()) {
                        scene->SunOrbitStart = sky["orbit_angle"][0];
                        scene->SunOrbitEnd = sky["orbit_angle"][1];
                        orbit = scene->SunOrbitStart;
                    } else {
                        orbit = sky["orbit_angle"];
                        scene->SunOrbitStart = orbit;
                        scene->SunOrbitEnd = orbit;
                    }
                }
                
                if (sky.contains("altitude")) {
                    if (sky["altitude"].is_array()) {
                        scene->SunAltitudeStart = sky["altitude"][0];
                        scene->SunAltitudeEnd = sky["altitude"][1];
                        altitude = scene->SunAltitudeStart;
                    } else {
                        altitude = sky["altitude"];
                        scene->SunAltitudeStart = altitude;
                        scene->SunAltitudeEnd = altitude;
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
                    auto tri = new Triangle(v0, v1, v2, mat->albedoColor);
                    tri->SetMaterial(mat);
                    
                    if (objData.contains("uvs"))
                    {
                        auto& uvs = objData["uvs"];
                        fvec2 uv0 = { uvs[0][0], uvs[0][1] };
                        fvec2 uv1 = { uvs[1][0], uvs[1][1] };
                        fvec2 uv2 = { uvs[2][0], uvs[2][1] };
                        tri->SetUVs(uv0, uv1, uv2);
                    }
                    
                    if (objData.contains("name")) tri->Name = objData["name"];
                    scene->AddGeometry(tri);
                }
                else if (type == "mesh")
                {
                    vector<Triangle> triangles;
                    auto mat = ParseMaterial(objData, scene.get(), textureCache);

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
                            fvec3 s = { 1.0f, 1.0f, 1.0f };
                            if (objData["scale"].is_array())
                            {
                                s = {
                                    objData["scale"][0].get<float>(),
                                    objData["scale"][1].get<float>(),
                                    objData["scale"][2].get<float>()
                                };
                            }
                            else if (objData["scale"].is_number())
                            {
                                float val = objData["scale"].get<float>();
                                s = { val, val, val };
                            }
                            transform = glm::scale(transform, s);
                            hasTransform = true;
                        }

                        if (hasTransform)
                        {
                            for (auto& part : loadedParts)
                            {
                                for (auto& tri : part.triangles)
                                {
                                    // Transform vertices
                                    tri.v0 = fvec3(transform * fvec4(tri.v0, 1.0f));
                                    tri.v1 = fvec3(transform * fvec4(tri.v1, 1.0f));
                                    tri.v2 = fvec3(transform * fvec4(tri.v2, 1.0f));
                                    
                                    // Recalculate normal
                                    fvec3 edge1 = tri.v1 - tri.v0;
                                    fvec3 edge2 = tri.v2 - tri.v0;
                                    tri.normal = glm::normalize(glm::cross(edge1, edge2));
                                    
                                    // Recalculate tangent
                                    tri.CalculateTangent();
                                }
                            }
                        }

                        for (const auto& part : loadedParts)
                        {
                            triangles.insert(triangles.end(), part.triangles.begin(), part.triangles.end());
                        }
                    }

                    if (objData.contains("triangles"))
                    {
                        for (const auto& triData : objData["triangles"])
                        {
                            fvec3 v0 = {
                                triData["v0"][0].get<float>(),
                                triData["v0"][1].get<float>(),
                                triData["v0"][2].get<float>()
                            };
                            fvec3 v1 = {
                                triData["v1"][0].get<float>(),
                                triData["v1"][1].get<float>(),
                                triData["v1"][2].get<float>()
                            };
                            fvec3 v2 = {
                                triData["v2"][0].get<float>(),
                                triData["v2"][1].get<float>(),
                                triData["v2"][2].get<float>()
                            };
                            fvec3 color = { 1.0f, 1.0f, 1.0f };
                            if (triData.contains("color"))
                            {
                                color = {
                                    triData["color"][0].get<float>(),
                                    triData["color"][1].get<float>(),
                                    triData["color"][2].get<float>()
                                };
                            }
                            
                            Triangle tri(v0, v1, v2, color);
                            
                            if (triData.contains("uvs"))
                            {
                                auto& uvs = triData["uvs"];
                                fvec2 uv0 = { uvs[0][0], uvs[0][1] };
                                fvec2 uv1 = { uvs[1][0], uvs[1][1] };
                                fvec2 uv2 = { uvs[2][0], uvs[2][1] };
                                tri.SetUVs(uv0, uv1, uv2);
                            }
                            
                            triangles.push_back(tri);
                        }
                    }
                    
                    // Assign material to all triangles
                    for(auto& tri : triangles)
                    {
                        tri.SetMaterial(mat);
                    }

                    auto mesh = new Mesh(triangles, mat);
                    if (objData.contains("name")) mesh->Name = objData["name"];
                    scene->AddGeometry(mesh);
                }
                else if (type == "cube")
                {
                    fvec3 center = { 0, 0, 0 };
                    if (objData.contains("center"))
                        center = { objData["center"][0], objData["center"][1], objData["center"][2] };
                        
                    float size = 100.0f;
                    if (objData.contains("size"))
                        size = objData["size"];
                        
                    glm::mat4 transform(1.0f);
                    if (objData.contains("transform"))
                    {
                        auto& t = objData["transform"];
                        // Assume row-major or column-major? GLM is column-major.
                        // If JSON is just a list of 16 numbers, usually row-major in export, but let's assume standard GLM construction.
                        // If 4x4 array:
                        if (t.size() == 4 && t[0].is_array())
                        {
                            for (int i = 0; i < 4; i++)
                                for (int j = 0; j < 4; j++)
                                    transform[i][j] = t[i][j]; // transform[col][row]
                        }
                        else if (t.size() == 16)
                        {
                            // Flat array
                             for (int i = 0; i < 16; i++)
                                transform[i / 4][i % 4] = t[i]; // check this mapping
                                // GLM mat4 constructor takes columns.
                                // If input is row-major (common in JSON), we need to transpose or fill carefully.
                                // Let's assume input is column-major flat array for now or just fill it.
                                // Actually, make_mat4 takes a pointer.
                                // Let's just iterate.
                                // transform[col][row] in GLM.
                                // If input is row-major: t[row*4 + col]
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

        // Load Animators
        if (j.contains("animators"))
        {
            for (const auto& animData : j["animators"])
            {
                Animator anim;
                anim.TargetName = animData["target"];
                anim.Property = animData["property"];
                if (animData.contains("normalized")) anim.NormalizedTime = animData["normalized"];
                
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
                
                scene->Animators.push_back(anim);
            }
        }

        return scene;
    }
};
