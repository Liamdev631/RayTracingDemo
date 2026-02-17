#pragma once
#include "Hit.h"

using namespace glm;

/**
 * @brief Ray with origin and direction in world space.
 */
struct Ray
{
public:
	/** @brief Ray origin in world space. */
	fvec3 position;
	/** @brief Normalized ray direction in world space. */
	fvec3 direction;

public:
	/** @brief Creates a default-initialized ray. */
	Ray() = default;
	/** @brief Destroys the ray. */
	~Ray() = default;
};

