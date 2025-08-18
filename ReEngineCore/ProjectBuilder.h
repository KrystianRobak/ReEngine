#pragma once

#include "CoreExport.h"
#include <fstream>
#include <vector>



class CORE_API ProjectBuilder
{
public:
	ProjectBuilder(const char* ProjectPath)
	{
		ProjectPath_ = ProjectPath;
	}

	void ParseConfig();


private:
	const char* ProjectPath_;
	std::vector<const char*> ModulesToLoad_;
};



//extern "C"
//{
//	CORE_API ProjectBuilder* CreateProjectBuilder();
//}

