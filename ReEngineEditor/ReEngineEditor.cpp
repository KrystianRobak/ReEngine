#include <windows.h>
#include <iostream>

#include "ReflectionEngine.h"
#include "ReflectionHelpers.h"

#include "ProjectBuilder.h"

#include "CoordinatorWrapper.h"
#include "ApplicationWrapper.h"
#include "Window.h"



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
    
    CoordinatorWrapper coordinatorWrap;
    ApplicationWrapper applicationWrap;



    coordinatorWrap.LoadFunctions(engineDLL);
    applicationWrap.LoadFunctions(engineDLL);

    if (!applicationWrap.IsProperlyLoaded() && !coordinatorWrap.IsProperlyLoaded()) {
        FreeLibrary(engineDLL);
        return -1;
    }

    


   

    applicationWrap.app = applicationWrap.CreateApplication();
    applicationWrap.Application_Init(applicationWrap.app);

    

    Window window;
    window.Init(1280, 720, "Okno zycia");

    Engine::IEngineApi* engine = static_cast<Engine::IEngineApi*>(coordinatorWrap.GetCoordinator());

    builder.ParseConfig(engine);

    auto systems = Reflection::Registry::Instance().GetAllSystems();

    for (auto system : systems)
    {
        LOGF_WARN("Znaleziono system: %s", system->fullName)
    }


    auto transform = Reflection::Registry::Instance().FindComponent("/Script/GeneratedModule.Transform");
    auto renderSystem = Reflection::Registry::Instance().FindSystem("/Script/GeneratedModule.RenderOpenGL");

    coordinatorWrap.RegisterComponent(transform);
    System* renderer = coordinatorWrap.RegisterSystem(renderSystem);
    renderer->Init(engine);
    Signature signature;
    signature.set(coordinatorWrap.GetComponentType(transform->fullName));

    coordinatorWrap.SetSystemSignature(renderSystem->fullName, signature);
    
    auto entity = coordinatorWrap.CreateEntity();

    coordinatorWrap.AddComponent(entity, transform->fullName);

    if (applicationWrap.app == nullptr) {
        LOGF_ERROR("%s", "Application pointer is uninitialized!")
    }


    while (window.is_running())
    {
        window.PreRender();
		
        window.Render();
        
        renderer->Update(0.016f);
        applicationWrap.Application_Update(applicationWrap.app);
        applicationWrap.Application_Render(applicationWrap.app);
        window.PostRender();
    }

    applicationWrap.DestroyApplication(applicationWrap.app);
    FreeLibrary(engineDLL);

    return 0;
}
