#pragma once
#include "Hit.h"

using namespace glm;

struct Ray
{
public:
	fvec3 position;
	fvec3 direction;

public:
	Ray() = default;
	~Ray() = default;
};

