#pragma once

#include "Api/IApplicationApi.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/AssetManagerApi.h"

#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>
#include <memory>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "ILayerManager.h"



class Commander;

class System
{
public:
	virtual void InitApi(Editor::IEngineEditorApi* engine, IApplicationApi* application, std::shared_ptr<AssetManagerApi> AssetManger = nullptr)
	{
		engine_ = engine;
		assetManager_ = AssetManger;
		application_ = application;

		OnInit();
	};

	virtual void OnInit() {};

	virtual void Update(float dt) = 0;

	void InjectCommander(std::shared_ptr<Commander> commander)
	{
		commander_ = commander;
	}

	void InjectLayerManager(ILayerManager* layerManager)
	{
		layerManager_ = layerManager;
	}

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}
	// --- Graph Dependencies (Populated by ProjectBuilder) ---
	std::string SystemName;
	std::vector<std::string> RunAfter;
	std::vector<std::string> RunBefore;
	std::vector<std::string> WriteComponents;
	bool RunOnMainThread = false; // Optional: Force main thread for Rendering/UI
	bool IsEditorSystem = false; // Optional: Only run in editor mode
protected:
	std::set<Entity> mEntities;
	Editor::IEngineEditorApi* engine_;
	IApplicationApi* application_;
	std::shared_ptr<Commander> commander_;
	std::shared_ptr<AssetManagerApi> assetManager_;
	ILayerManager* layerManager_ = nullptr;
};
