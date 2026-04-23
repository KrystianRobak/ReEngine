#pragma once

#include "../ComponentBasic.h"

REFCOMPONENT()
struct Transform
{
	REFVARIABLE()
	glm::vec3 position = glm::vec3(0.0f);
	
	REFVARIABLE()
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	REFVARIABLE()
	glm::vec3 scale = glm::vec3(1.0f);
};
