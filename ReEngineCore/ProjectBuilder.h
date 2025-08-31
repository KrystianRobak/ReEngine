#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>

#include "System/System.h"

class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(const char* ProjectPath)
	{
		ProjectPath_ = ProjectPath;
	}

	void ParseConfig(Editor::IEngineEditorApi* engine);

	System* LoadModule(const char* ModuleName);


	const char* ProjectPath_;
	std::vector<const char*> ModulesToLoad_;
	System* RendererSystem_;
	System* PhysicsSystem_;
};



//extern "C"
//{
//	CORE_API ProjectBuilder* CreateProjectBuilder();
//}
