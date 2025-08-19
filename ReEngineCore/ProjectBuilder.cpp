#include "ProjectBuilder.h"

typedef System* (*CreateSystemFunc)();

void ProjectBuilder::ParseConfig() {
    ModulesToLoad_.push_back("Systems/OpenGLRenderer.dll");

	for(const char* moduleName : ModulesToLoad_) {
		LOGF_INFO("Loading module: %s", moduleName);
		System* system = LoadModule(moduleName);
		system->Init();
		RendererSystem_ = system; // Store the renderer system if needed
	}
}

System* ProjectBuilder::LoadModule(const char* ModuleName)
{
	HMODULE SystemDll = LoadLibraryA(ModuleName);
	if (!SystemDll) {
		LOGF_ERROR("Failed to load module: %s", ModuleName);
		return nullptr;
	}
	
	auto createFunc = reinterpret_cast<CreateSystemFunc>(GetProcAddress(SystemDll, "CreateSystem"));
	if (!createFunc) {
		LOGF_ERROR("Failed to find CreateSystem function in module: %s", ModuleName);
		return nullptr;
	}

	LOGF_INFO("Module loaded successfully: %s", ModuleName);
	return createFunc();
}

