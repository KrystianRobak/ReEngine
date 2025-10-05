#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>

#include "System/System.h"
#include "Logger.h"

class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(std::string ProjectPath)
	{
		ProjectPath_ = ProjectPath;

		if (checkAndRemovePrefix("--game-dll="))
		{
			LOGF_INFO("Successfully loaded project path")
		}
		else 
		{
			LOGF_ERROR("Failed during loading project path")
		}
	}

	void ParseConfig(Editor::IEngineEditorApi* engine);

	System* LoadModule(const char* ModuleName);

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
};



//extern "C"
//{
//	CORE_API ProjectBuilder* CreateProjectBuilder();
//}
