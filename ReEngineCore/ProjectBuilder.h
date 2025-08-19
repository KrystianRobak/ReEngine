#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>
#include <windows.h>
#include "Logger.h"
#include <System/System.h>


class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(const char* ProjectPath)
	{
		ProjectPath_ = ProjectPath;
	}

	void ParseConfig();

	System* LoadModule(const char* ModuleName);

public:
	const char* ProjectPath_;
	std::vector<const char*> ModulesToLoad_;
	System* RendererSystem_ = nullptr; // Store the renderer system if needed
	System* PhysicsSystem_ = nullptr; // Store the physics system if needed
	System* GameplaySystem_ = nullptr; // Store the gameplay system if needed
};



//extern "C"
//{
//	CORE_API ProjectBuilder* CreateProjectBuilder();
//}

