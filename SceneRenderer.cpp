#include "SceneRenderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include "Geometry.h"

using namespace glm;

SceneRenderer::SceneRenderer(const sf::Vector2u& frameSize)
	: _frameSize(frameSize), _frameBuffer(nullptr)
{

}

SceneRenderer::~SceneRenderer()
{

}

void SceneRenderer::SetScene(shared_ptr<Scene> scene) noexcept
{
	_currentScene = scene;
}

#define RecursionDepth 10

void SceneRenderer::Render(unique_ptr<sf::Texture>& renderTarget)
{
	Ray currentRay = Ray();
	Hit outHit;
	const auto halfFrameSize = _frameSize / 2U;
	fvec3 cumulativeColor;
	sf::Image buffer;
	buffer.create(_frameSize.x, _frameSize.y, sf::Color::Black);
	int index = 0;
	for (unsigned y = 0; y < _frameSize.y; y++)
		for (unsigned x = 0; x < _frameSize.x; x++)
		{
			//currentRay.direction = normalize(fvec3(x - halfFrameSize.x, -10, y - halfFrameSize.y));
			currentRay.position = { (float)x - halfFrameSize.x, halfFrameSize.y - (float)y, -500 };
			//static float timer = 0.f;
			//timer += 0.01f;
			//currentRay.position = glm::rotateY(currentRay.position, timer);
			
			currentRay.direction = { 0, 0, 1 };
			//currentRay.direction = glm::rotateY(currentRay.direction, timer);

			float blendAmount = 1.f;
			cumulativeColor = { 0, 0, 0 };
			for (int depth = 0; depth < RecursionDepth; depth++)
			{
				bool hit = _currentScene.lock()->Intersects(currentRay, outHit);
				if (!hit) // Stop the recursion when we miss
					break;
				cumulativeColor += outHit.Color * blendAmount;
				blendAmount *= 0.3f;

				// Reflect the ray off of the surface
				currentRay.position = outHit.Position + outHit.Normal * 0.01f;
				currentRay.direction = outHit.Normal;
			}
			buffer.setPixel(x, y, Color_GLM_To_SF(cumulativeColor));
		}

	renderTarget->loadFromImage(buffer);
	//return tex;
}
