#pragma once

#include "../ComponentBasic.h"

REFLECT()
struct Transform
{
	REFVARIABLE()
	glm::vec3 position;
	
	REFVARIABLE()
	glm::quat rotation;

	REFVARIABLE()
	glm::vec3 scale;
};
