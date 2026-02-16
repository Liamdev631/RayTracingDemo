#include "SceneRenderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include "Geometry.h"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

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
	sf::Image buffer;
	buffer.create(_frameSize.x, _frameSize.y, sf::Color::Black);
	
	const unsigned int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	const unsigned int rowsPerThread = _frameSize.y / numThreads;

	// We need a way to set pixels concurrently. sf::Image isn't thread-safe for setPixel if close by? 
	// Actually sf::Image::setPixel just writes to a vector. As long as x,y are distinct, it should be fine.
	// But sf::Image operations are not guaranteed thread safe. 
	// Safer to write to a raw buffer then load.
	std::vector<sf::Uint8> rawPixels(_frameSize.x * _frameSize.y * 4);

	auto renderRow = [&](unsigned startY, unsigned endY) {
		Ray currentRay = Ray();
		Hit outHit;
		const auto halfFrameSize = _frameSize / 2U;
		fvec3 cumulativeColor;
		
		auto scene = _currentScene.lock();
		if (!scene) return;

		for (unsigned y = startY; y < endY; y++)
		{
			for (unsigned x = 0; x < _frameSize.x; x++)
			{
				currentRay.position = { (float)x - halfFrameSize.x, halfFrameSize.y - (float)y, -500 };
				currentRay.direction = { 0, 0, 1 };

				float blendAmount = 1.f;
				cumulativeColor = { 0, 0, 0 };
				
				// Local ray for recursion to avoid modifying the loop variable if we reused it
				Ray traceRay = currentRay;

				for (int depth = 0; depth < RecursionDepth; depth++)
				{
					bool hit = scene->Intersects(traceRay, outHit);
					if (!hit) 
						break;
					
					cumulativeColor += outHit.Color * blendAmount;
					blendAmount *= 0.3f;

					traceRay.position = outHit.Position + outHit.Normal * 0.01f;
					traceRay.direction = outHit.Normal;
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
