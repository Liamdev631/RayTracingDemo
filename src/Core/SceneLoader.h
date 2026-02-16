#pragma once
#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "Scene.h"
#include "GeometrySphere.h"
#include "GeometryPlane.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace std;

class SceneLoader
{
public:
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

        // Load Ambient Light
        if (j.contains("ambient_light"))
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

        // Load Scene Settings (Camera & Animation)
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
        }

        // Load Lights
        bool hasDirectionalLight = false;
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
                    scene->AddLight(new PointLight(pos, color, intensity));
                }
                else if (type == "directional")
                {
                    if (hasDirectionalLight)
                    {
                        cerr << "Warning: Multiple directional lights found. Only the first one will be used." << endl;
                        continue;
                    }
                    
                    fvec3 dir = {
                        lightData["direction"][0].get<float>(),
                        lightData["direction"][1].get<float>(),
                        lightData["direction"][2].get<float>()
                    };
                    fvec3 color = {
                        lightData["color"][0].get<float>(),
                        lightData["color"][1].get<float>(),
                        lightData["color"][2].get<float>()
                    };
                    float intensity = lightData["intensity"].get<float>();
                    
                    scene->SunLight = DirectionalLight(dir, color, intensity);
                    scene->InitialSunDirection = glm::normalize(dir);
                    scene->InitialSunIntensity = intensity;
                    hasDirectionalLight = true;
                }
            }
        }
        
        if (!hasDirectionalLight)
        {
            cerr << "Warning: No directional light found in scene. Using default." << endl;
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
                    scene->AddGeometry(new GeometrySphere(pos, radius));
                }
                else if (type == "plane")
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
                    scene->AddGeometry(new GeometryPlane(origin, normal));
                }
            }
        }

        return scene;
    }
};
