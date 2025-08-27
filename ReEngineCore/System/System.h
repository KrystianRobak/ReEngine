#pragma once

#include "SystemApi/CoordinatorSystemApi.h"
#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>



class System
{
public:
	virtual void Init(Engine::IEngineApi* engine) = 0;

	virtual void Update(float dt) = 0;

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}

protected:
	std::set<Entity> mEntities;
	Engine::IEngineApi* engine_;
};
