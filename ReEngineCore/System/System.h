#pragma once

#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/AssetManagerApi.h"

#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>
#include <memory>

#include "GL/glew.h"
#include "GLFW/glfw3.h"



class Commander;

class System
{
public:
	virtual void InitApi(Editor::IEngineEditorApi* engine, std::shared_ptr<AssetManagerApi> AssetManger = nullptr)
	{
		engine_ = engine;
		assetManager_ = AssetManger;
	};

	virtual void Update(float dt) = 0;

	void InjectCommander(std::shared_ptr<Commander> commander)
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
	std::shared_ptr<Commander> commander_;
	std::shared_ptr<AssetManagerApi> assetManager_;
};
