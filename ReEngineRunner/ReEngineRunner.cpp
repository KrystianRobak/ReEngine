#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "Api/IApplicationApi.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "ProjectBuilder.h"
#include "Logger.h"
#include "json/json.hpp" // Include JSON to parse the config
#include "GameView.h"

namespace fs = std::filesystem;
using json = nlohmann::json;
using FuncPtr = void* (*)();

// Helper to read INI values
std::string GetIniValue(const std::string& filepath, const std::string& key) {
    if (!fs::exists(filepath)) return "";
    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + "=") == 0) {
            // Returns the part after the "="
            return line.substr(key.length() + 1);
        }
    }
    return "";
}

int main(int argc, char** argv)
{
    // 1. Load the Core Engine DLL
    HMODULE engineDLL = LoadLibraryA("ReEngine.dll");
    if (!engineDLL) {
        MessageBoxA(NULL, "Failed to load ReEngine.dll", "Fatal Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // 2. Create Application Instance
    FuncPtr createFunc = (FuncPtr)(GetProcAddress(engineDLL, "CreateApplication"));
    if (!createFunc) {
        MessageBoxA(NULL, "Entry point CreateApplication not found!", "Fatal Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    IApplicationApi* app = static_cast<IApplicationApi*>(createFunc());
    app->Init();
    Editor::IEngineEditorApi* engine = app->GetCoordinatorEditor();

    // 3. Locate Configuration File (from Game.ini)
    std::string configFileName = GetIniValue("Game.ini", "Config");
    std::string startScene = GetIniValue("Game.ini", "StartScene");

    // Fallback search if Game.ini is missing
    if (configFileName.empty()) {
        for (const auto& entry : fs::directory_iterator(fs::current_path())) {
            if (entry.path().extension() == ".json") {
                configFileName = entry.path().string();
                break;
            }
        }
    }

    if (configFileName.empty()) {
        MessageBoxA(NULL, "No project configuration (.json) found!", "Error", MB_OK);
        return -1;
    }

    // 4. Parse Config to get the Game Name
    // We need the "name" field (e.g. "stopa") to know which DLL to load ("stopa.dll")
    std::string gameName = "Game"; // Default fallback
    std::ifstream confFile(configFileName);
    if (confFile.good()) {
        try {
            json j;
            confFile >> j;
            if (j.contains("name")) {
                gameName = j["name"];
            }
        }
        catch (const std::exception& e) {
            LOGF_ERROR("Failed to parse config JSON: %s", e.what());
        }
    }

    // 5. Construct Game DLL Name
    // ProjectBuilder expects the path to the DLL (e.g., "stopa.dll")
    std::string gameDllPath = gameName + ".dll";

    if (!fs::exists(gameDllPath)) {
        std::string msg = "Could not find Game DLL: " + gameDllPath;
        MessageBoxA(NULL, msg.c_str(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    LOGF_INFO("Runner starting project: %s using config %s", gameDllPath.c_str(), configFileName.c_str());

    ProjectBuilder builder(gameDllPath, engine, app);
    builder.ParseConfig();

    app->InitSystems();

    if (!startScene.empty()) {
        std::replace(startScene.begin(), startScene.end(), '\\', '/');
    }

    app->SetState(ApplicationState::Play);
    app->StartGameThreads();


    engine->AddEventListener(Events::Engine::LayerManager::INITIALIZED,
        [app, &builder, engine, startScene](Event& e) {
            ILayerManager* layerManager = app->GetLayerManager();

            builder.SetLayerManager(layerManager);
            builder.InjectLayerManager();

            layerManager->AddLayer<GameLayer>();

            engine->OpenScene(startScene);

            app->GetCoordinatorEditor()->SendEvent(Events::Application::CAMERA_CHANGED);
        });

    while (app->IsRunning()) {
        app->CoordinationLoop();
    }

    delete app;
    FreeLibrary(engineDLL);
    return 0;
}