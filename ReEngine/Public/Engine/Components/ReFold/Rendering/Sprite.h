#pragma once

#include "../ComponentBasic.h"

REFCOMPONENT()
struct Sprite
{
	REFVARIABLE()
		glm::vec3 position;

	REFVARIABLE()
		glm::quat rotation;

	REFVARIABLE()
		glm::vec3 scale;
};