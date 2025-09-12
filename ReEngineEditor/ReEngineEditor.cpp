#include <windows.h>
#include <iostream>
#include <memory>

#include "EngineApi/CoordinatorEditorApi.h"
#include "IApplicationApi.h"

#include "Logger.h"
#include "ReflectionEngine.h"
#include "ReflectionHelpers.h"

#include "ProjectBuilder.h"

#include "CoordinatorWrapper.h"
#include "ApplicationWrapper.h"


#include "Panels/AddingPanel.h"
#include "Panels/AnimationPanel.h"
#include "Panels/ControlPanel.h"
#include "Panels/FileBrowser.h"
#include "Panels/ItemsSelectionPanel.h"
#include "Panels/KeyframeEditorPanel.h"
#include "Panels/PropertyPanel.h"
#include "Panels/SceneView.h"


using FuncPtr = void* (*)();

int main(int argc, char** argv)
{
    for(int i = 1; i < argc;i++)
    {
        LOGF_INFO("Passed as argument: %s", argv[i])
    }

    ProjectBuilder builder(argv[1]);

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

    LOGF_INFO("Module loaded successfully");
    IApplicationApi* Application = static_cast<IApplicationApi*>(createFunc());

    CoordinatorWrapper coordinatorWrap;
    ApplicationWrapper applicationWrap;



    coordinatorWrap.LoadFunctions(engineDLL);
    applicationWrap.LoadFunctions(engineDLL);

    if (!applicationWrap.IsProperlyLoaded() && !coordinatorWrap.IsProperlyLoaded()) {
        FreeLibrary(engineDLL);
        return -1;
    }

    Application->Init();


    Application->SetCreateUiPanels([&]() { 
        Application->AddUIComponent(new SceneView());
        Application->AddUIComponent(new AddingPanel());

        Application->AddUIComponent(new ControlPanel());
        Application->AddUIComponent(new FileBrowser());
        Application->AddUIComponent(new ItemsSelectionPanel());
        Application->AddUIComponent(new PropertyPanel());

		});


    Editor::IEngineEditorApi* engine = Application->GetCoordinatorEditor();

    builder.ParseConfig(engine);

    auto systems = Reflection::Registry::Instance().GetAllSystems();

    for (auto system : systems)
    {
        LOGF_WARN("Znaleziono system: %s", system->fullName)
    }

    auto transform = Reflection::Registry::Instance().FindComponent("/Script/GeneratedModule.Transform");
	auto sprite = Reflection::Registry::Instance().FindComponent("/Script/GeneratedModule.Sprite");
    auto renderSystem = Reflection::Registry::Instance().FindSystem("/Script/GeneratedModule.RenderOpenGL");
	auto physicsSystem = Reflection::Registry::Instance().FindSystem("/Script/GeneratedModule.Physics3D");

    coordinatorWrap.RegisterComponent(transform, true);
	coordinatorWrap.RegisterComponent(sprite);

    System* renderer = coordinatorWrap.RegisterSystem(renderSystem);

	//renderer->InitApi(engine, glfwGetCurrentContext());

    Signature signature;
    signature.set(coordinatorWrap.GetComponentType(transform->fullName));

    coordinatorWrap.SetSystemSignature(renderSystem->fullName, signature);

    System* physics = coordinatorWrap.RegisterSystem(physicsSystem);

    physics->InitApi(engine, glfwGetCurrentContext());

    Signature signature2;
    signature2.set(coordinatorWrap.GetComponentType(transform->fullName));


    coordinatorWrap.SetSystemSignature(physicsSystem->fullName, signature2);


	Application->InitSystems();

	Application->StartThreads();

    engine->CreateEntity();


    while (Application->IsRunning())
    {
		Application->Update();
    }

    delete Application;

    FreeLibrary(engineDLL);

    return 0;
}
