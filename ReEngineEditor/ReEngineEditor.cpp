#include <windows.h>
#include <iostream>
#include <memory>

#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/IApplicationApi.h"
#include "ILayerManager.h"

#include "Logger.h"
#include "ReflectionEngine.h"
#include "ReflectionHelpers.h"

#include "ProjectBuilder.h"

#include <thread>
#include "EditorLayer.h"
#include <sstream>
#include <GuizmoLayer.h>


using FuncPtr = void* (*)();

int main(int argc, char** argv)
{
    std::cout << argv[1] << std::endl;

    


	LOGF_INFO("Loading Engine.dll");
    HMODULE engineDLL = LoadLibraryA("ReEngine.dll");
    if (!engineDLL) {
        LOGF_ERROR("%s","Failed to load Engine.dll")
        return -1;
    }

    FuncPtr createFunc = (FuncPtr)(GetProcAddress(engineDLL, "CreateApplication"));
    if (!createFunc) {
        LOGF_ERROR("Failed to find CreateSystem function in module");
        return -1;
    }

    LOGF_INFO("Engine loaded successfully");

    IApplicationApi* Application = static_cast<IApplicationApi*>(createFunc());

    Application->Init();

    Editor::IEngineEditorApi* engine = Application->GetCoordinatorEditor();

    ProjectBuilder builder(argv[1], engine);

    builder.ParseConfig();


	Application->InitSystems();

    engine->AddEventListener(Events::Engine::LayerManager::INITIALIZED, [Application, &builder](Event& e) {
        ILayerManager* layerManager = Application->GetLayerManager();

		builder.SetLayerManager(layerManager);
		builder.InjectLayerManager();

        layerManager->AddLayerThreadSafe<EditorLayer>();
		layerManager->AddLayerThreadSafe<GuizmoLayer>();
        });

	Application->StartGameThreads();



    while (Application->IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    delete Application;

    FreeLibrary(engineDLL);

    return 0;
}
