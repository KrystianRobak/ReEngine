#include "ProjectBuilder.h"

#include <Windows.h>

#include <Logger.h>



typedef System* (*CreateSystemFunc)();

void ProjectBuilder::ParseConfig(Editor::IEngineEditorApi* engine) {
	const char* moduleName = "Systems/OpenGLRenderer.dll";

	LOGF_INFO("Loading module: %s", moduleName);
	System* system = LoadModule(moduleName);
	RendererSystem_ = system; // Store the renderer system if needed

	moduleName = "Systems/Physics3D.dll";

	LOGF_INFO("Loading module: %s", moduleName);
	System* system2 = LoadModule(moduleName);
	PhysicsSystem_ = system2; // Store the renderer system if needed

    ModulesToLoad_.push_back("Systems/OpenGLRenderer.dll");
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

