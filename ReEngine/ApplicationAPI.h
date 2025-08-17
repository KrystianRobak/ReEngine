#pragma once
#include "ReEngineExport.h"

extern "C" {
    ENGINE_API void* CreateApplication();
    ENGINE_API void DestroyApplication(void* app);

    ENGINE_API void Application_Init(void* app);
    ENGINE_API void Application_Update(void* app);
    ENGINE_API void Application_Render(void* app);
    ENGINE_API bool Application_IsRunning(void* app);

}