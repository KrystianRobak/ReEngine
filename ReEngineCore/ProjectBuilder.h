#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>

#include "System/System.h"
#include "Logger.h"
#include "json/json.hpp"

namespace fs = std::filesystem;

class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(std::string ProjectPath, Editor::IEngineEditorApi* engine)
	{
		ProjectPath_ = ProjectPath;
		engineAPI_ = engine;

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
		for(auto& System : SystemsLoaded_)
		{
			System->InjectLayerManager(layerManager_);
		}
	}

	void ParseConfig();

	System* LoadModule(const char* ModuleName);

	void LoadTextures(nlohmann::json& config, fs::path projectPath);

	bool checkAndRemovePrefix(const std::string& prefix) {
		// 1. Check if ProjectPath_ begins with the prefix
		if (ProjectPath_.rfind(prefix, 0) == 0) {
			// 2. If it matches, erase the prefix from the beginning
			ProjectPath_.erase(0, prefix.length());
			return true; // Return true as a match was found and removed
		}
		return false; // Return false, no match found
	}

	std::string ProjectPath_;
	std::vector<const char*> ModulesToLoad_;
	System* RendererSystem_;
	System* PhysicsSystem_;
	std::vector<System*> SystemsLoaded_;
	Editor::IEngineEditorApi* engineAPI_ = nullptr;

public:
	ILayerManager* layerManager_ = nullptr;
};



//extern "C"
//{
//	CORE_API ProjectBuilder* CreateProjectBuilder();
//}
