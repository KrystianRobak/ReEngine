#pragma once

#include "../ComponentBasic.h"

struct Transform
{
	REFVARIABLE(x)
	glm::vec3 position;
	
	REFVARIABLE(x)
	glm::quat rotation;

	REFVARIABLE(x)
	glm::vec3 scale;
};
