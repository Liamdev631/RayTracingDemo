#include "SceneRenderer.h"
#include "../Core/Constants.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Geometry.h"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

using namespace glm;

SceneRenderer::SceneRenderer(const sf::Vector2u& frameSize)
	: _frameSize(frameSize)
{
}

SceneRenderer::~SceneRenderer()
{
}

void SceneRenderer::SetScene(shared_ptr<Scene> scene) noexcept
{
	_currentScene = scene;
}

void SceneRenderer::Render(unique_ptr<sf::Texture>& renderTarget)
{
	sf::Image buffer;
	buffer.create(_frameSize.x, _frameSize.y, sf::Color::Black);
	const unsigned int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	const unsigned int rowsPerThread = _frameSize.y / numThreads;
	auto scene = _currentScene.lock();
	if (!scene) return;
	// Camera setup
	glm::vec3 cameraPosition = scene->CameraPosition;
	glm::vec3 cameraTarget = scene->CameraTarget;
	float fov = scene->CameraFOV;
	glm::vec3 up(0, 1, 0);
	glm::vec3 forward = glm::normalize(cameraTarget - cameraPosition);
	// Handle case where forward is parallel to up (e.g. looking straight down)
	if (glm::abs(glm::dot(forward, up)) > 0.99f)
		up = glm::vec3(0, 0, 1); // Use Z as up if looking along Y
	glm::vec3 right = glm::normalize(glm::cross(forward, up));
	glm::vec3 cameraUp = glm::cross(right, forward);
	float aspectRatio = (float)_frameSize.x / (float)_frameSize.y;
	float scale = tan(glm::radians(fov * 0.5f));
	// Safer to write to a raw buffer then load.
	std::vector<sf::Uint8> rawPixels(_frameSize.x * _frameSize.y * 4);
	auto renderRow = [&](unsigned startY, unsigned endY) {
		Ray currentRay = Ray();
		Hit outHit;
		fvec3 cumulativeColor;
		fvec3 throughput;
		for (unsigned y = startY; y < endY; y++)
		{
			for (unsigned x = 0; x < _frameSize.x; x++)
			{
				// Calculate ray direction
				// NDC coordinates (0 to 1) -> (-1 to 1)
				float x_ndc = (2.0f * (x + 0.5f) / _frameSize.x - 1.0f) * aspectRatio * scale;
				float y_ndc = (1.0f - 2.0f * (y + 0.5f) / _frameSize.y) * scale; // Flip Y for image coordinates
				currentRay.position = cameraPosition;
				currentRay.direction = glm::normalize(forward + right * x_ndc + cameraUp * y_ndc);
				cumulativeColor = { 0, 0, 0 };
				throughput = { 1, 1, 1 };
				// Local ray for recursion
				Ray traceRay = currentRay;
				for (int depth = 0; depth < Constants::MAX_BOUNCE_COUNT; depth++)
				{
					bool hit = scene->Intersects(traceRay, outHit);
					if (!hit) 
					{
						// Sky color (very simple gradient or constant)
						cumulativeColor += throughput * fvec3(0.1f, 0.1f, 0.3f);
						break;
					}
					// PBR Attributes Retrieval
					fvec3 albedo = outHit.Color;
					fvec3 normal = outHit.Normal;
					float roughness = outHit.Roughness;
					float metallic = outHit.Metallic;
					if (outHit.HitGeometry)
					{
						outHit.HitGeometry->GetPBR(outHit.Position, albedo, roughness, metallic, normal);
					}
					// Direct Lighting Accumulator
					fvec3 directLight = { 0, 0, 0 };
					// Ambient
					directLight += albedo * scene->AmbientLightColor * scene->AmbientIntensity;
					// Directional Light (Sun)
					fvec3 lightDir = -scene->SunLight.Direction; // Direction TO light
					float diff = glm::max(glm::dot(normal, lightDir), 0.0f);
					// Shadow check for directional light
					bool inShadow = false;
					Ray shadowRay;
					shadowRay.position = outHit.Position + normal * Constants::SHADOW_BIAS; // Use geometric normal for bias
					shadowRay.direction = lightDir;
					if (scene->IntersectsAny(shadowRay))
						inShadow = true;
					if (!inShadow)
					{
						// Diffuse
						// For metals, diffuse is 0 (all light is specular reflection)
						fvec3 diffuseColor = albedo * (1.0f - metallic);
						directLight += diffuseColor * scene->SunLight.Color * scene->SunLight.Intensity * diff;
						// Specular (Blinn-Phong)
						fvec3 viewDir = glm::normalize(-traceRay.direction);
						fvec3 halfwayDir = glm::normalize(lightDir + viewDir);
						// Map roughness to specular power
						float specPower = glm::mix(128.0f, 2.0f, roughness * roughness); 
						float spec = glm::pow(glm::max(glm::dot(normal, halfwayDir), 0.0f), specPower);
						// Specular color
						// Dielectric: White (0.04)
						// Metal: Albedo
						fvec3 specColor = glm::mix(fvec3(0.04f), albedo, metallic);
						directLight += specColor * scene->SunLight.Color * scene->SunLight.Intensity * spec;
					}
					// Point Lights
					for (auto* light : scene->GetLightCollection())
					{
						fvec3 L = light->Position - outHit.Position;
						float dist = glm::length(L);
						L = glm::normalize(L);
						float atten = 1.0f / (1.0f + 0.1f * dist + 0.01f * dist * dist);
						float pDiff = glm::max(glm::dot(normal, L), 0.0f);
						Ray pShadowRay;
						pShadowRay.position = outHit.Position + normal * Constants::SHADOW_BIAS;
						pShadowRay.direction = L;
						Hit shadowHit;
						bool pInShadow = false;
						if (scene->Intersects(pShadowRay, shadowHit))
						{
							if (shadowHit.Distance < dist)
								pInShadow = true; // In shadow
						}
						if (!pInShadow)
						{
							// Diffuse
							fvec3 diffuseColor = albedo * (1.0f - metallic);
							directLight += diffuseColor * light->Color * light->Intensity * pDiff * atten;
                            // Specular
                            fvec3 viewDir = glm::normalize(-traceRay.direction);
                            fvec3 halfwayDir = glm::normalize(L + viewDir);
                            float specPower = glm::mix(128.0f, 2.0f, roughness * roughness);
                            float spec = glm::pow(glm::max(glm::dot(normal, halfwayDir), 0.0f), specPower);
                            fvec3 specColor = glm::mix(fvec3(0.04f), albedo, metallic);
                            directLight += specColor * light->Color * light->Intensity * spec * atten;
						}
					}
                    // Accumulate direct lighting
                    cumulativeColor += throughput * directLight;
					// Calculate Fresnel for reflection
                    fvec3 F0 = glm::mix(fvec3(0.04f), albedo, metallic);
                    fvec3 viewDir = glm::normalize(-traceRay.direction);
                    float cosTheta = glm::clamp(glm::dot(normal, viewDir), 0.0f, 1.0f);
                    fvec3 F = F0 + (fvec3(1.0f) - F0) * std::pow(1.0f - cosTheta, 5.0f);
                    // Update throughput for next bounce (Reflection)
                    // Attenuate by roughness to simulate microfacet loss in single-ray approximation
                    throughput *= F * (1.0f - roughness);
					if (glm::length(throughput) < 0.01f) break;
					traceRay.position = outHit.Position + normal * Constants::SHADOW_BIAS;
					traceRay.direction = glm::reflect(traceRay.direction, normal);
				}
				sf::Color c = Color_GLM_To_SF(cumulativeColor);
				unsigned int index = (x + y * _frameSize.x) * 4;
				rawPixels[index] = c.r;
				rawPixels[index + 1] = c.g;
				rawPixels[index + 2] = c.b;
				rawPixels[index + 3] = c.a;
			}
		}
	};
	for (unsigned int i = 0; i < numThreads; ++i)
	{
		unsigned int startY = i * rowsPerThread;
		unsigned int endY = (i == numThreads - 1) ? _frameSize.y : (startY + rowsPerThread);
		threads.emplace_back(renderRow, startY, endY);
	}
	for (auto& t : threads)
	{
		t.join();
	}
	buffer.create(_frameSize.x, _frameSize.y, rawPixels.data());
	renderTarget->loadFromImage(buffer);
}
