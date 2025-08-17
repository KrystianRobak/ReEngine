#pragma once

#include "CoreExport.h"
#include <vector>



class CORE_API ProjectBuilder
{

	void ParseConfig();


private:
	std::vector<const char*> ModulesToLoad;
};



extern "C"
{
	CORE_API ProjectBuilder* CreateProjectBuilder();
}

