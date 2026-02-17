#include "ProjectBuilder.h"

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <random>

#include <Logger.h>

#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/IApplicationApi.h"
#include "ILayerManager.h"
#include "ReflectionEngine.h"

typedef System* (*CreateSystemFunc)();

inline std::vector<std::string> splitBracedList(const std::string& input) {
    std::vector<std::string> result;
    std::string trimmed = input;
    if (!trimmed.empty() && trimmed.front() == '{') trimmed.erase(trimmed.begin());
    if (!trimmed.empty() && trimmed.back() == '}') trimmed.pop_back();

    std::stringstream ss(trimmed);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), token.end());
        if (!token.empty()) result.push_back(token);
    }
    return result;
}

std::string GenerateTempName(const std::string& originalName) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 9999);
    return originalName + "_hotreload_" + std::to_string(dis(gen));
}

std::string ProjectBuilder::CreateShadowCopy(const std::string& originalPath)
{
    fs::path sourcePath(originalPath);
    if (!fs::exists(sourcePath)) {
        LOGF_ERROR("Cannot find module to copy: %s", originalPath.c_str());
        return "";
    }

    std::string stem = sourcePath.stem().string();
    std::string extension = sourcePath.extension().string();
    fs::path directory = sourcePath.parent_path();

    std::string tempFileName = GenerateTempName(stem);
    fs::path targetDllPath = directory / (tempFileName + extension);

    try {
        fs::copy_file(sourcePath, targetDllPath, fs::copy_options::overwrite_existing);
        LOGF_INFO("[HotReload] Shadow copy created: %s", targetDllPath.string().c_str());
    }
    catch (std::filesystem::filesystem_error& e) {
        LOGF_ERROR("[HotReload] Failed to copy DLL: %s", e.what());
        return "";
    }

    fs::path sourcePdbPath = directory / (stem + ".pdb");
    fs::path targetPdbPath = directory / (tempFileName + ".pdb");

    if (fs::exists(sourcePdbPath)) {
        try {
            fs::copy_file(sourcePdbPath, targetPdbPath, fs::copy_options::overwrite_existing);
            LOGF_INFO("[HotReload] PDB Shadow copy created.");
        }
        catch (std::filesystem::filesystem_error& e) {
            LOGF_WARN("[HotReload] Failed to copy PDB (Debugging might be limited): %s", e.what());
        }
    }
    return targetDllPath.string();
}

void ProjectBuilder::ParseConfig(bool isPackaged) {
    fs::path projectPath(ProjectPath_);

    LOGF_INFO("Loading game module: %s", ProjectPath_.c_str());

    std::string moduleToLoad = ProjectPath_;

    if (!isPackaged) {
        std::string shadowPath = CreateShadowCopy(ProjectPath_);

        if (!shadowPath.empty()) {
            moduleToLoad = shadowPath;
            CurrentTempDLLPath_ = shadowPath;
            LOGF_INFO("[HotReload] Switched to shadow copy: %s", shadowPath.c_str());
        }
    }

    if (!ModuleLoader_.Load(moduleToLoad, "GameModule")) {
        LOGF_ERROR("Failed to load GameModule dll at: %s", moduleToLoad.c_str());
    }
    else {
        LOGF_INFO("Game module loaded successfully");
    }

    std::string projectName = projectPath.filename().string();
    std::string projectNameSliced = projectName.substr(0, projectName.length() - 4);

    if (projectPath.filename() == projectName) projectPath = projectPath.parent_path();
    if (projectPath.filename() == "Debug" || projectPath.filename() == "debug") projectPath = projectPath.parent_path();
    if (projectPath.filename() == "bin") projectPath = projectPath.parent_path();

    fs::path configPath = projectPath / (projectNameSliced + ".json");
    LOGF_INFO("Loading project config: %s", configPath.string().c_str());

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

    LOGF_INFO("Project config: %s, loaded sucessfully", configPath.string().c_str());

    auto LoadPersistentModule = [&](const std::string& name, const std::string& path) -> System* {

        if (ModuleLoader_.IsLoaded(path)) {

            return nullptr;
        }
        LOGF_INFO("Loading module: %s", path.c_str());
        return LoadModule(path.c_str());
        };

    if (config.contains("Renderer")) {
        std::string rendererDll = "Systems/" + config["Renderer"].get<std::string>() + ".dll";
        System* sys = LoadPersistentModule("Renderer", rendererDll);
        if (sys) RendererSystem_ = sys;
    }

    if (config.contains("Physics")) {
        std::string physicsDll = "Systems/" + config["Physics"].get<std::string>() + ".dll";
        System* sys = LoadPersistentModule("Physics", physicsDll);
        if (sys) PhysicsSystem_ = sys;
    }

    if (config.contains("ModulesToLoad")) {
        for (const auto& module : config["ModulesToLoad"]) {
            std::string moduleDll = "Systems/" + module.get<std::string>() + ".dll";
            LoadPersistentModule(module.get<std::string>(), moduleDll);
        }
    }

    LoadTextures(config, projectPath);

    auto systems = Reflection::Registry::Instance().GetAllSystems();
    auto components = Reflection::Registry::Instance().GetAllComponents();

    for (auto component : components)
    {
        if (std::strcmp(component->name, "Transform") == 0)
            engineAPI_->RegisterComponent(component, true);
        else
            engineAPI_->RegisterComponent(component, false);
    }

    for (auto system : systems)
    {
        System* existingSystem = engineAPI_->GetSystem(system->fullName);

        System* registeredSystem = nullptr;

        if (existingSystem) {

            registeredSystem = existingSystem;
        }
        else {
            LOGF_INFO("Registering system into coordinator %s", system->fullName);
            registeredSystem = engineAPI_->RegisterSystem(system);
            if (registeredSystem == nullptr) continue;

            registeredSystem->InitApi(engineAPI_, applicationAPI_, engineAPI_->GetAssetManager());
            SystemsLoaded_.push_back(registeredSystem);
        }

        registeredSystem->SystemName = system->fullName;

        Signature signature;
        LOGF_INFO("Setting up system: %s", system->fullName)

        for (auto variable : system->variables)
        {
            if (std::strcmp(variable.name, "ComponentsToRegister") == 0) {
                std::vector<std::string> components = splitBracedList(variable.defaultValue);
                for (std::string component : components) {
                    signature.set(engineAPI_->GetComponentType(component));
                }
            }
            else if (std::strcmp(variable.name, "SystemsToRunAfter") == 0) {
                std::vector<std::string> deps = splitBracedList(variable.defaultValue);
                for (std::string dep : deps) registeredSystem->RunAfter.push_back(dep);
            }
            else if (std::strcmp(variable.name, "SystemsToRunBefore") == 0) {
                std::vector<std::string> deps = splitBracedList(variable.defaultValue);
                for (std::string dep : deps) registeredSystem->RunBefore.push_back(dep);
            }
            else if (std::strcmp(variable.name, "WriteComponents") == 0) {
                std::vector<std::string> comps = splitBracedList(variable.defaultValue);
                for (std::string c : comps) registeredSystem->WriteComponents.push_back(c);
            }
            else if (std::strcmp(variable.name, "RunOnMainThread") == 0) {
                if (variable.defaultValue == "true" || variable.defaultValue == "1")
                    registeredSystem->RunOnMainThread = true;
            }
        }
        engineAPI_->SetSystemSignature(system->fullName, signature);
    }
}

System* ProjectBuilder::LoadModule(const char* ModuleName)
{
    if (!ModuleLoader_.Load(ModuleName, ModuleName)) {
        LOGF_ERROR("ModuleLoader failed to load: %s", ModuleName);
        return nullptr;
    }

    HMODULE SystemDll = ModuleLoader_.GetHandle(ModuleName);
    if (!SystemDll) {
        LOGF_ERROR("ModuleLoader reported success, but handle is null for: %s", ModuleName);
        return nullptr;
    }

    auto createFunc = reinterpret_cast<CreateSystemFunc>(GetProcAddress(SystemDll, "CreateSystem"));
    if (!createFunc) {
        LOGF_ERROR("Failed to find 'CreateSystem' export in module: %s", ModuleName);
        return nullptr;
    }

    return createFunc();
}

void ProjectBuilder::LoadTextures(nlohmann::json& config, fs::path projectPath)
{
    namespace fs = std::filesystem;
    if (config.contains("TexturesFolders") && engineAPI_->GetAssetManager().get()) {
        auto* assetManager = engineAPI_->GetAssetManager().get();
        for (const auto& folderEntry : config["TexturesFolders"]) {
            std::string folderName = folderEntry.get<std::string>();
            fs::path textureFolderPath = projectPath / folderName;
            if (fs::exists(textureFolderPath) && fs::is_directory(textureFolderPath)) {
                for (const auto& entry : fs::recursive_directory_iterator(textureFolderPath)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".retex" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
                            assetManager->GetTexture(entry.path().generic_string());
                        }
                    }
                }
            }
        }
    }
}

void ProjectBuilder::CleanupGameModule()
{
    LOGF_INFO("[ProjectBuilder] Starting GameModule cleanup...");

    engineAPI_->PrepareForReload();

    SystemsLoaded_.clear();

    LOGF_INFO("[ProjectBuilder] Unregistering 'GameModule' from reflection registry...");
    Reflection::Registry::Instance().UnregisterModule("GameModule");

    LOGF_INFO("[ProjectBuilder] Unloading 'GameModule' DLL...");
    ModuleLoader_.Unload("GameModule");

    if (!CurrentTempDLLPath_.empty() && fs::exists(CurrentTempDLLPath_))
    {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            fs::remove(CurrentTempDLLPath_);
            fs::path pdbPath = fs::path(CurrentTempDLLPath_).replace_extension(".pdb");
            if (fs::exists(pdbPath)) fs::remove(pdbPath);
            LOGF_INFO("[HotReload] Cleaned up shadow copy.");
        }
        catch (std::filesystem::filesystem_error& e) {
            LOGF_WARN("[HotReload] Failed to cleanup shadow copy: %s", e.what());
        }
        CurrentTempDLLPath_.clear();
    }
}

void ProjectBuilder::UnloadAllModules()
{
    ModuleLoader_.UnloadAll();
    if (!CurrentTempDLLPath_.empty() && fs::exists(CurrentTempDLLPath_)) {
        fs::remove(CurrentTempDLLPath_);
    }
}

void ProjectBuilder::ReloadModules()
{
    LOGF_INFO("[ProjectBuilder] Starting targeted GameModule reload...");

    CleanupGameModule();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    ParseConfig();

    engineAPI_->RestoreAfterReload();

    if (layerManager_)
    {
        InjectLayerManager();
    }

    LOGF_INFO("[ProjectBuilder] Targeted reload complete!");
}