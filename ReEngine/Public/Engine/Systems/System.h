#pragma once

#include "Engine/Core/Types.h"
#include <set>


class System
{
public:
	virtual void Init() = 0;

	virtual void Update(float dt) = 0;

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}

protected:
	std::set<Entity> mEntities;
};
