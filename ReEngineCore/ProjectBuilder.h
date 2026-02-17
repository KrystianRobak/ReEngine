#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>

#include "System/System.h"
#include "Logger.h"
#include "json/json.hpp"
#include <ModuleLoader.h>

namespace fs = std::filesystem;

class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(std::string ProjectPath, Editor::IEngineEditorApi* engine, IApplicationApi* applicationApi)
	{
		ProjectPath_ = ProjectPath;
		engineAPI_ = engine;
		applicationAPI_ = applicationApi;

		if (checkAndRemovePrefix("--game-dll="))
		{
			LOGF_INFO("Successfully loaded project path")
		}
		else
		{
			LOGF_ERROR("Failed during loading project path")
		}
	}

	void SetLayerManager(ILayerManager* layerManager)
	{
		layerManager_ = layerManager;
	}

	void InjectLayerManager()
	{
		for (auto& System : SystemsLoaded_)
		{
			System->InjectLayerManager(layerManager_);
		}
	}

	void ParseConfig(bool isPackaged = false);

	System* LoadModule(const char* ModuleName);

	void LoadTextures(nlohmann::json& config, fs::path projectPath);

	void CleanupGameModule();

	void ReloadModules();
	void UnloadAllModules();

	std::string CreateShadowCopy(const std::string& originalPath);

	bool checkAndRemovePrefix(const std::string& prefix) {
		if (ProjectPath_.rfind(prefix, 0) == 0) {
			ProjectPath_.erase(0, prefix.length());
			return true;
		}
		return false;
	}

	std::string ProjectPath_;
	std::vector<const char*> ModulesToLoad_;
	System* RendererSystem_;
	System* PhysicsSystem_;
	std::vector<System*> SystemsLoaded_;
	Editor::IEngineEditorApi* engineAPI_ = nullptr;
	IApplicationApi* applicationAPI_ = nullptr;
	ModuleLoader ModuleLoader_;
	std::string CurrentTempDLLPath_;

public:
	ILayerManager* layerManager_ = nullptr;
};