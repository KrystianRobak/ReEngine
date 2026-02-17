#include "ModuleLoader.h"

ModuleLoader::~ModuleLoader()
{
    for (auto& [name, mod] : Modules_) {
        if (mod.isLoaded) {
            Unload(name);
        }
    }
}

bool ModuleLoader::Load(const std::string& dllPath, const std::string& moduleName)
{
    if (Modules_.count(moduleName) && Modules_[moduleName].isLoaded) {
        LOGF_WARN("Module '%s' is already loaded.", moduleName.c_str())
        return true;
    }

    HMODULE hLib = LoadLibraryA(dllPath.c_str());
    if (!hLib) {
        LOGF_ERROR("Failed to load DLL: %s (Windows Error: %lu)", dllPath.c_str(), GetLastError())
        return false;
    }

    Module mod;
    mod.libraryHandle = hLib;
    mod.dllPath = dllPath;
    mod.moduleName = moduleName;
    mod.isLoaded = true;

    Modules_[moduleName] = mod;

    LOGF_REGISTER_PASS("Module '%s' initialized and registered.", moduleName.c_str())
    return true;
}

void ModuleLoader::Unload(const std::string& moduleName)
{
    auto it = Modules_.find(moduleName);
    if (it == Modules_.end() || !it->second.isLoaded) {
        LOGF_WARN("Attempted to unload module '%s' which is not loaded.", moduleName.c_str())
        return;
    }

    if (FreeLibrary(it->second.libraryHandle)) {
        it->second.isLoaded = false;
        it->second.libraryHandle = nullptr;
        LOGF_INFO("Successfully unloaded: %s", moduleName.c_str())
    }
    else {
        LOGF_ERROR("FreeLibrary failed for '%s' (Error: %lu)", moduleName.c_str(), GetLastError())
    }
}

bool ModuleLoader::Reload(const std::string& moduleName)
{
    auto it = Modules_.find(moduleName);
    if (it == Modules_.end()) {
        LOGF_ERROR("Reload failed: Module '%s' was never registered.", moduleName.c_str())
        return false;
    }

    std::string cachedPath = it->second.dllPath;
    LOGF_INFO("Reloading module: %s...", moduleName.c_str())

    Unload(moduleName);
    return Load(cachedPath, moduleName);
}

void ModuleLoader::UnloadAll()
{
    for (auto& [name, mod] : Modules_) {
        if (mod.isLoaded) {
            if (name == "ReEngine")
                continue;
            Unload(name);
        }
    }
}
