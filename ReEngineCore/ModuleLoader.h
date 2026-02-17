#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>
#include "Logger.h"

struct Module {
    HMODULE libraryHandle = nullptr;
    std::string dllPath;
    std::string moduleName;
    bool isLoaded = false;
};

class ModuleLoader {
public:
    ModuleLoader() = default;
    ~ModuleLoader();

    ModuleLoader(const ModuleLoader&) = delete;
    ModuleLoader& operator=(const ModuleLoader&) = delete;

    bool Load(const std::string& dllPath, const std::string& moduleName);
    void Unload(const std::string& moduleName);
    bool Reload(const std::string& moduleName);
    void UnloadAll();

    bool IsLoaded(const std::string& moduleName) const {
        auto it = Modules_.find(moduleName);
        return (it != Modules_.end()) ? it->second.isLoaded : false;
    }

    HMODULE GetHandle(const std::string& moduleName) const {
        auto it = Modules_.find(moduleName);
        return (it != Modules_.end()) ? it->second.libraryHandle : nullptr;
    }

private:
    std::unordered_map<std::string, Module> Modules_;
};