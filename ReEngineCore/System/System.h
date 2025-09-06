#pragma once

#include "EngineApi/CoordinatorEditorApi.h"
#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>
#include "../../ReEngine/Commander.h"

class System
{
public:
	virtual void InitApi(Editor::IEngineEditorApi* engine)
	{
		engine_ = engine;
	};

	virtual void Update(float dt) = 0;

	void InjectCommander(Commander* commander)
	{
		commander_ = commander;
	}

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}

protected:
	std::set<Entity> mEntities;
	Editor::IEngineEditorApi* engine_;
	Commander* commander_;
};
