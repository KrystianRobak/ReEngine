#include "ProjectBuilder.h"

#include <Windows.h>
#include <filesystem>
#include <fstream>

#include <Logger.h>
#include "json/json.hpp"

typedef System* (*CreateSystemFunc)();

inline  std::vector<std::string> splitBracedList(const std::string& input) {
    std::vector<std::string> result;

    // Remove the braces { }
    std::string trimmed = input;
    if (!trimmed.empty() && trimmed.front() == '{') trimmed.erase(trimmed.begin());
    if (!trimmed.empty() && trimmed.back() == '}') trimmed.pop_back();

    std::stringstream ss(trimmed);
    std::string token;

    // Split by comma
    while (std::getline(ss, token, ',')) {
        // Trim leading/trailing spaces
        token.erase(token.begin(),
            std::find_if(token.begin(), token.end(),
                [](unsigned char ch) { return !std::isspace(ch); }));
        token.erase(std::find_if(token.rbegin(), token.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(),
            token.end());

        if (!token.empty())
            result.push_back(token);
    }

    return result;
}

void ProjectBuilder::ParseConfig() {
    namespace fs = std::filesystem;

    fs::path projectPath(ProjectPath_);

    // 1. Remove "bin/debug" if present
	LOGF_INFO("Loading game module: %s", ProjectPath_.c_str())

    HMODULE gameModule = LoadLibraryA(ProjectPath_.c_str());
    if (!gameModule) {
        LOGF_ERROR("%s", "Failed to load GameModule dll")
    }

	LOGF_INFO("Game module loaded successfully")

    std::string projectName = projectPath.filename().string();

    std::string projectNameSliced = projectName.substr(0, projectName.length() - 4);

    if (projectPath.filename() == projectName) {
        projectPath = projectPath.parent_path(); // remove "Debug"
    }
    if (projectPath.filename() == "Debug" || projectPath.filename() == "debug") {
        projectPath = projectPath.parent_path(); // remove "Debug"
    }
    if (projectPath.filename() == "bin") {
        projectPath = projectPath.parent_path(); // remove "bin"
    }

    // 3. Build full path to config file
    fs::path configPath = projectPath / (projectNameSliced + ".json");

    LOGF_INFO("Loading project config: %s", configPath.string().c_str());

    // 4. Parse config
    nlohmann::json config;
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            LOGF_ERROR("Could not open config file: %s", configPath.string().c_str());
            return;
        }
        file >> config;
    }
    catch (const nlohmann::json::parse_error& e) {
        LOGF_ERROR("Failed to parse config file: %s (%s)", configPath.string().c_str(), e.what());
        return;
    }

	LOGF_INFO("Config file loaded successfully");

    // 5. Load modules from config
    if (config.contains("Renderer")) {
        std::string rendererDll = "Systems/" + config["Renderer"].get<std::string>() + ".dll";
        LOGF_INFO("Loading module: %s", rendererDll.c_str());
        RendererSystem_ = LoadModule(rendererDll.c_str());
    }

    if (config.contains("Physics")) {
        std::string physicsDll = "Systems/" + config["Physics"].get<std::string>() + ".dll";
        LOGF_INFO("Loading module: %s", physicsDll.c_str());
        PhysicsSystem_ = LoadModule(physicsDll.c_str());
    }

    if(config.contains("ModulesToLoad")) {
        for (const auto& module : config["ModulesToLoad"]) {
            std::string moduleDll = "Systems/" + module.get<std::string>() + ".dll";
            LOGF_INFO("Loading module: %s", moduleDll.c_str());
            System* system = LoadModule(moduleDll.c_str());
        }
	}


    auto systems = Reflection::Registry::Instance().GetAllSystems();
    auto components = Reflection::Registry::Instance().GetAllComponents();

    for (auto component : components)
    {
        engineAPI_->RegisterComponent(component);
    }

    for (auto system : systems)
    {
        System* registeredSystem = engineAPI_->RegisterSystem(system);
		SystemsLoaded_.push_back(registeredSystem);
        registeredSystem->InitApi(engineAPI_);

        Signature signature;

        LOGF_INFO("Setting up system: %s", system->fullName)

            for (auto variable : system->variables)
            {

                if (std::strcmp(variable.name, "ComponentsToRegister") == 0)
                {
                    std::vector<std::string> components = splitBracedList(variable.defaultValue);

                    for (std::string component : components)
                    {
                        LOGF_INFO("Registered %s to %s", component.c_str(), system->fullName)
                            signature.set(engineAPI_->GetComponentType(component));
                    }
                }
            }

        engineAPI_->SetSystemSignature(system->fullName, signature);

        LOGF_INFO("System %s setup complete", system->fullName)
    }

    LOGF_INFO("Config parsing complete");
}

System* ProjectBuilder::LoadModule(const char* ModuleName)
{
    HMODULE SystemDll = LoadLibraryA(ModuleName);
    if (!SystemDll) {
        LOGF_ERROR("Failed to load module: %s", ModuleName);
        return nullptr;
    }

    auto createFunc = reinterpret_cast<CreateSystemFunc>(GetProcAddress(SystemDll, "CreateSystem"));
    if (!createFunc) {
        LOGF_ERROR("Failed to find CreateSystem function in module: %s", ModuleName);
        return nullptr;
    }

    LOGF_INFO("Module loaded successfully: %s", ModuleName);
    return createFunc();
}
