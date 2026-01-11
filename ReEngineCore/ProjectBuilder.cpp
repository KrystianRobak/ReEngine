#include "ProjectBuilder.h"

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <Logger.h>


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

	LoadTextures(config, projectPath);

    auto systems = Reflection::Registry::Instance().GetAllSystems();
    auto components = Reflection::Registry::Instance().GetAllComponents();

    for (auto component : components)
    {
        if(std::strcmp(component->name, "Transform") == 0)
            engineAPI_->RegisterComponent(component, true);
        else
			engineAPI_->RegisterComponent(component, false);
    }

    for (auto system : systems)
    {
        bool isEditorOnly = false;
        // 2. Register System
        System* registeredSystem = engineAPI_->RegisterSystem(system);
        SystemsLoaded_.push_back(registeredSystem);

        registeredSystem->InitApi(engineAPI_,applicationAPI_ ,engineAPI_->GetAssetManager());

        // 3. Set Internal Name for Graph
        registeredSystem->SystemName = system->fullName;
        if (isEditorOnly)
        {
            registeredSystem->IsEditorSystem = true;
        }
           

        Signature signature;
        LOGF_INFO("Setting up system: %s", system->fullName)

            // 4. Parse Variables
            for (auto variable : system->variables)
            {
                // A. Components
                if (std::strcmp(variable.name, "ComponentsToRegister") == 0)
                {
                    std::vector<std::string> components = splitBracedList(variable.defaultValue);
                    for (std::string component : components) {
                        LOGF_INFO("Registered %s to %s", component.c_str(), system->fullName)
                            signature.set(engineAPI_->GetComponentType(component));
                    }
                }

                // B. RunAfter
                else if (std::strcmp(variable.name, "SystemsToRunAfter") == 0)
                {
                    std::vector<std::string> deps = splitBracedList(variable.defaultValue);
                    for (std::string dep : deps) {
                        registeredSystem->RunAfter.push_back(dep);
                    }
                }

                // C. RunBefore
                else if (std::strcmp(variable.name, "SystemsToRunBefore") == 0)
                {
                    std::vector<std::string> deps = splitBracedList(variable.defaultValue);
                    for (std::string dep : deps) {
                        registeredSystem->RunBefore.push_back(dep);
                    }
                }

                // D. WriteComponents
                else if (std::strcmp(variable.name, "WriteComponents") == 0)
                {
                    std::vector<std::string> comps = splitBracedList(variable.defaultValue);
                    for (std::string c : comps) registeredSystem->WriteComponents.push_back(c);
                }

                // E. RunOnMainThread
                else if (std::strcmp(variable.name, "RunOnMainThread") == 0)
                {
                    if (variable.defaultValue == "true" || variable.defaultValue == "1")
                        registeredSystem->RunOnMainThread = true;
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

void ProjectBuilder::LoadTextures(nlohmann::json& config, fs::path projectPath)
{
    namespace fs = std::filesystem;
    if (config.contains("TexturesFolders")) {

        if (auto* assetManager = engineAPI_->GetAssetManager().get()) {

            for (const auto& folderEntry : config["TexturesFolders"]) {
                std::string folderName = folderEntry.get<std::string>();
                fs::path textureFolderPath = projectPath / folderName;

                if (fs::exists(textureFolderPath) && fs::is_directory(textureFolderPath)) {
                    LOGF_INFO("Scanning asset folder (Recursive): %s", textureFolderPath.string().c_str());

                    // --- THE CHANGE IS HERE ---
                    // Use recursive_directory_iterator to traverse all subfolders
                    for (const auto& entry : fs::recursive_directory_iterator(textureFolderPath)) {

                        // Check if the entry is a regular file (not a subdirectory)
                        if (entry.is_regular_file()) {
                            // ... (Rest of file extension check logic remains the same) ...
                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
                                fs::path relativePath = fs::relative(entry.path(), projectPath);
                                std::string assetPath = relativePath.generic_string();

                                LOGF_INFO("Pre-loading texture: %s", assetPath.c_str());
                                assetManager->GetTexture(assetPath);
                            }
                        }
                    }
                }
                else {
                    LOGF_ERROR("Configured texture folder does not exist: %s", textureFolderPath.string().c_str());
                }
            }
        }
        else {
            LOGF_ERROR("Failed to acquire AssetManager during Project Setup.");
        }
    }
}
