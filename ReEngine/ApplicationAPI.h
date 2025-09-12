#pragma once
#include "ReEngineExport.h"
#include "Public/Engine/Core/Application.h"

extern "C" {

    ENGINE_API void* CreateApplication() {
        return static_cast<void*>(new Application());
    }
}