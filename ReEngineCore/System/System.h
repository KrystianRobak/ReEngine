#pragma once

#include "EngineApi/CoordinatorEditorApi.h"
#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>



class System
{
public:
	virtual void InitApi(Editor::IEngineEditorApi* engine)
	{
		engine_ = engine;
	};

	virtual void Update(float dt) = 0;

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}

protected:
	std::set<Entity> mEntities;
	Editor::IEngineEditorApi* engine_;
};
