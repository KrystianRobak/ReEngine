#include <windows.h>
#include <iostream>
#include <memory>

#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/IApplicationApi.h"
#include "ILayerManager.h"

#include "Logger.h"
#include "ReflectionEngine.h"
#include "ReflectionHelpers.h"

#include "ProjectBuilder.h"

#include "CoordinatorWrapper.h"
#include "ApplicationWrapper.h"

#include <thread>
#include "EditorLayer.h"
#include <sstream>
#include <GuizmoLayer.h>


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

using FuncPtr = void* (*)();

int main(int argc, char** argv)
{
    //std::string input;
    //std::cin >> input;

    //ProjectBuilder builder(input);
    ProjectBuilder builder(argv[1]);


	LOGF_INFO("Loading Engine.dll");
    HMODULE engineDLL = LoadLibraryA("ReEngine.dll");
    if (!engineDLL) {
        LOGF_ERROR("%s","Failed to load Engine.dll")
        return -1;
    }

    FuncPtr createFunc = (FuncPtr)(GetProcAddress(engineDLL, "CreateApplication"));
    if (!createFunc) {
        LOGF_ERROR("Failed to find CreateSystem function in module");
        return -1;
    }

    LOGF_INFO("Engine loaded successfully");

    IApplicationApi* Application = static_cast<IApplicationApi*>(createFunc());

    CoordinatorWrapper coordinatorWrap;
    ApplicationWrapper applicationWrap;



    coordinatorWrap.LoadFunctions(engineDLL);
    applicationWrap.LoadFunctions(engineDLL);

    if (!applicationWrap.IsProperlyLoaded() && !coordinatorWrap.IsProperlyLoaded()) {
        FreeLibrary(engineDLL);
        return -1;
    }

    Application->Init();

    Editor::IEngineEditorApi* engine = Application->GetCoordinatorEditor();

    builder.ParseConfig(engine);

    auto systems = Reflection::Registry::Instance().GetAllSystems();
	auto components = Reflection::Registry::Instance().GetAllComponents();

    for (auto component : components)
    {
        coordinatorWrap.RegisterComponent(component);
    }

    for (auto system : systems)
    {
		System* registeredSystem = coordinatorWrap.RegisterSystem(system);

		registeredSystem->InitApi(engine);

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
                    signature.set(coordinatorWrap.GetComponentType(component));
                }
            }
        }

        coordinatorWrap.SetSystemSignature(system->fullName, signature);

		LOGF_INFO("System %s setup complete", system->fullName)
    }

	Application->InitSystems();

    engine->AddEventListener(Events::Engine::LayerManager::INITIALIZED, [Application](Event& e) {
        ILayerManager* layerManager = Application->GetLayerManager();
        layerManager->AddLayer<EditorLayer>();
		layerManager->AddLayer<GuizmoLayer>();
        });

	Application->StartThreads();



    while (Application->IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    delete Application;

    FreeLibrary(engineDLL);

    return 0;
}
