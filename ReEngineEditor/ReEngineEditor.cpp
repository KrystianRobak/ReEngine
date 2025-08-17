#include <windows.h>
#include <iostream>

#include "ReflectionEngine.h"
#include "ReflectionHelpers.h"

#include "CoordinatorWrapper.h"
#include "ApplicationWrapper.h"
#include "Window.h"



int main()
{
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

    auto transform = Reflection::Registry::Instance().FindComponent("/Script/GeneratedModule.Transform");

    applicationWrap.app = applicationWrap.CreateApplication();
    applicationWrap.Application_Init(applicationWrap.app);

    coordinatorWrap.RegisterComponent(transform);

    Window window;
    window.Init(300, 300, "Okno zycia");

    

    if (applicationWrap.app == nullptr) {
        LOGF_ERROR("%s", "Application pointer is uninitialized!")
    }

    
    auto registry = Reflection::Registry::Instance();
    auto Classes = Reflection::Registry::Instance().GetAllSystems();

    for (auto* cls : Classes)
    {
        InspectClass(cls);
        std::cout << "\n";
    }

    Classes = Reflection::Registry::Instance().GetAllComponents();

    for (auto* cls : Classes)
    {
        InspectClass(cls);
        std::cout << "\n";
    }

    while (window.is_running())
    {
        window.PreRender();
        window.Render();
        applicationWrap.Application_Update(applicationWrap.app);
        applicationWrap.Application_Render(applicationWrap.app);
        window.PostRender();
    }

    applicationWrap.DestroyApplication(applicationWrap.app);
    FreeLibrary(engineDLL);

    return 0;
}
