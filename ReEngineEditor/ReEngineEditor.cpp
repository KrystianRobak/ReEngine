#include <windows.h>
#include <iostream>

#include "CoordinatorWrapper.h"

typedef void* (*CreateAppFunc)();
typedef void (*DestroyAppFunc)(void*);
typedef void (*AppInitFunc)(void*);
typedef void (*AppUpdateFunc)(void*);
typedef void (*AppRenderFunc)(void*);
typedef bool (*AppIsRunningFunc)(void*);


int main()
{
    HMODULE engineDLL = LoadLibraryA("ReEngine.dll");
    if (!engineDLL) {
        std::cerr << "Failed to load Engine.dll\n";
        return -1;
    }

    auto CreateApplication = (CreateAppFunc)GetProcAddress(engineDLL, "CreateApplication");
    auto DestroyApplication = (DestroyAppFunc)GetProcAddress(engineDLL, "DestroyApplication");
    auto Application_Init = (AppInitFunc)GetProcAddress(engineDLL, "Application_Init");
    auto Application_Update = (AppUpdateFunc)GetProcAddress(engineDLL, "Application_Update");
    auto Application_Render = (AppRenderFunc)GetProcAddress(engineDLL, "Application_Render");
    auto Application_IsRunning = (AppIsRunningFunc)GetProcAddress(engineDLL, "Application_IsRunning");

	auto Coordinator_CreateEntity = (CoordinatorCreateEntityFunc)GetProcAddress(engineDLL, "Coordinator_CreateEntity");
	auto Coordinator_CreateLightEntity = (CoordinatorCreateLightEntityFunc)GetProcAddress(engineDLL, "Coordinator_CreateLightEntity");
	auto Coordinator_DestroyEntity = (CoordinatorDestroyEntityFunc)GetProcAddress(engineDLL, "Coordinator_DestroyEntity");
	auto Coordinator_GetEntitiesAmount = (CoordinatorGetEntitiesAmountFunc)GetProcAddress(engineDLL, "Coordinator_GetEntitiesAmount");
	auto Coordinator_GetLightEntitiesAmount = (CoordinatorGetLightEntitiesAmountFunc)GetProcAddress(engineDLL, "Coordinator_GetLightEntitiesAmount");
	auto Coordinator_SetSelectedEntity = (CoordinatorSetSelectedEntityFunc)GetProcAddress(engineDLL, "Coordinator_SetSelectedEntity");
	auto Coordinator_GetSelectedEntity = (CoordinatorGetSelectedEntityFunc)GetProcAddress(engineDLL, "Coordinator_GetSelectedEntity");
	auto Coordinator_GetComponentTypes = (CoordinatorGetComponentTypesFunc)GetProcAddress(engineDLL, "Coordinator_GetComponentTypes");  

	CoordinatorWrapper Wrapper = CoordinatorWrapper(
		Coordinator_CreateEntity,
		Coordinator_CreateLightEntity,
		Coordinator_DestroyEntity,
		Coordinator_GetEntitiesAmount,
		Coordinator_GetLightEntitiesAmount,
		Coordinator_SetSelectedEntity,
		Coordinator_GetSelectedEntity,
		Coordinator_GetComponentTypes
	);

    if (!CreateApplication || !DestroyApplication || !Application_Init || !Application_Update || !Application_Render || !Application_IsRunning) {
        std::cerr << "Failed to load one or more engine functions\n";
        FreeLibrary(engineDLL);
        return -1;
    }

    void* app = CreateApplication();

    if (app == nullptr) {
        std::cerr << "Pointer is uninitialized!" << std::endl;
    }

    Application_Init(app);

    while (Application_IsRunning(app)) {
        Application_Update(app);
        Application_Render(app);
    }

    DestroyApplication(app);
    FreeLibrary(engineDLL);

    return 0;
}
