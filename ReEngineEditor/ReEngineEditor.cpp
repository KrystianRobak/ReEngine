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


void SetupHotReloadSystem(IApplicationApi* app, ProjectBuilder& builder, Editor::IEngineEditorApi* engine)
{
    engine->AddEventListener(Events::Engine::LayerManager::INITIALIZED,
        [app, &builder](Event& e) {
            ILayerManager* layerManager = app->GetLayerManager();

            builder.SetLayerManager(layerManager);
            builder.InjectLayerManager();

            layerManager->AddLayerThreadSafe<EditorLayer>();
            layerManager->AddLayerThreadSafe<GuizmoLayer>();
        });

    engine->AddEventListener(Events::Application::RECOMPILE_READY,
        [app, &builder](Event& e) {
            LOGF_INFO("[HotReload] Received RECOMPILE_READY event");

            builder.ReloadModules();

            app->RestartAfterRecompile();

            LOGF_INFO("[HotReload] Hot-reload complete!");
        });
}

int main(int argc, char** argv)
{

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

    ProjectBuilder builder(argv[1], engine, Application);

    builder.ParseConfig();


	Application->InitSystems();

    SetupHotReloadSystem(Application, builder, engine);

	Application->StartGameThreads();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (Application->IsRunning())
    {
        Application->CoordinationLoop();
    }

    delete Application;

    FreeLibrary(engineDLL);

    return 0;
}
