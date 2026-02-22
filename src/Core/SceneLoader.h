#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>
#include "Scene.h"
#include "Material.h"

// Forward declarations
namespace sf {
    class Image;
}

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
    static const sf::Image* GetOrLoadTexture(Scene* scene, std::map<string, int>& cache, const string& path);

    /**
     * @brief Parses a material from a JSON object or loads it from a file.
     * @param j The JSON object containing material properties or a reference to a material file.
     * @param scene The scene to load textures into.
     * @param cache A cache mapping texture paths to texture indices.
     * @return A shared pointer to the parsed material.
     */
    static shared_ptr<PBRMaterial> ParseMaterial(const json& j, Scene* scene, std::map<string, int>& cache);

public:
    /**
     * @brief Loads a scene from a JSON file on disk.
     * @param filename Path to the scene file.
     * @return Scene instance or nullptr on failure.
     */
    static shared_ptr<Scene> LoadScene(const string& filename);
};
