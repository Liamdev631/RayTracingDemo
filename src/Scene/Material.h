#pragma once
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

using fvec3 = glm::vec3;

/**
 * @brief PBR material inputs and optional texture maps.
 *
 * Textures are stored as raw SFML images owned by the scene.
 */
struct PBRMaterial
{
    /** @brief Base albedo color used when no albedo map is provided. */
    fvec3 albedoColor = { 1.0f, 1.0f, 1.0f };
    /** @brief Optional albedo texture map. */
    const sf::Image* albedoMap = nullptr;
    /** @brief Optional normal map in tangent space. */
    const sf::Image* normalMap = nullptr;
    /** @brief Scalar roughness value used when no roughness map is provided. */
    float roughnessVal = 0.5f;
    /** @brief Optional roughness texture map. */
    const sf::Image* roughnessMap = nullptr;
    /** @brief Scalar metallic value used when no metallic map is provided. */
    float metallicVal = 0.0f;
    /** @brief Optional metallic texture map. */
    const sf::Image* metallicMap = nullptr;
    /** @brief Optional gloss map where roughness = 1 - gloss. */
    const sf::Image* glossMap = nullptr;
};
