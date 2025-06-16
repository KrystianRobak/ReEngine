#pragma once

#include "../System.h"
#include <Engine/Core/Coordinator/Coordinator.h>
#include <Engine/Components/BehaviourScript.h>
#include "../../../../Script.h"
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <windows.h>

const std::vector<std::string> EngineIncludePaths = {
    "C:\\Users\\ragberr\\Desktop\\ReEngine\\ReEngine\\Public", // Example: C:/mingw64/home/user/MyEngine/include
    "C:\\Users\\ragberr\\Desktop\\ReEngine\\ReEngine\\Public\\Engine\\Components",     // Example: C:/mingw64/home/user/MyEngine/include/Engine/Components
};


class BehaviourSystem : public System {
public:
    void Init() override {
        coordinator = Coordinator::GetCoordinator();

        coordinator->AddEventListener(METHOD_LISTENER_NO_PARAM(Events::Application::TOGGLE, BehaviourSystem::LoadScripts));
    }

    void Update(float dt) override {
        for (auto const& script : Scripts) {
            script.second->Update();
        }
    }

    bool CompileScript(const std::string& scriptPath, const std::string& outputDir, const std::vector<std::string>& includePaths) {
        std::filesystem::path sourcePath(scriptPath);
        if (!std::filesystem::exists(sourcePath)) {
            std::cerr << "[Error] Script source file not found: " << scriptPath << std::endl;
            return false;
        }

        std::filesystem::path outputDirPath(outputDir);
        if (!std::filesystem::exists(outputDirPath)) {
            try {
                std::filesystem::create_directories(outputDirPath);
                std::cout << "[Info] Created output directory: " << outputDir << std::endl;
            }
            catch (const std::filesystem::filesystem_error& ex) {
                std::cerr << "[Error] Failed to create output directory: " << outputDir << " - " << ex.what() << std::endl;
                return false;
            }
        }

        std::string scriptName = sourcePath.stem().string();
        std::string outputFileName = scriptName + ".dll";
        std::string outputDllPath = (outputDirPath / outputFileName).string();

        // --- Construct the g++ compile command for Windows DLL ---
        // g++ [options] input_file -o output_file
        std::string compileCommand = "g++ ";

        // Standard C++20
        compileCommand += "-std=c++20 ";
        // Compile as a shared library (DLL on Windows)
        compileCommand += "-shared ";
        // Output file name and path
        compileCommand += "-o \"" + outputDllPath + "\" "; // Use quotes around path in case of spaces
        // Input source file
        compileCommand += "\"" + scriptPath + "\" "; // Use quotes around path

        // Add include paths (-I for g++)
        for (const auto& path : includePaths) {
            compileCommand += "-I\"" + path + "\" "; // Use quotes around path
        }

        // Add any necessary linker flags here if linking against external libraries
        // (Usually not needed for simple scripts only using the Script base class)
        // compileCommand += "-L\"Path/To/Your/Lib\" -lYourLib ";


        std::cout << "[Info] Compiling script: " << scriptPath << std::endl;
        std::cout << "[Info] Compile command: " << compileCommand << std::endl;

        // --- Execute the command and capture output using _popen ---
        FILE* pipe = _popen(compileCommand.c_str(), "r");
        if (!pipe) {
            std::cerr << "[Error] Failed to execute compile command: " << compileCommand << std::endl;
            return false;
        }

        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            // Print compiler output to stderr
            std::cerr << "Compiler Output: " << buffer;
        }

        // Get the exit code of the command using _pclose
        int result = _pclose(pipe);

        // On Windows with _pclose, the return value is the exit code
        if (result == 0) {
            std::cout << "[Info] Successfully compiled script: " << scriptPath << " -> " << outputDllPath << std::endl;
            return true;
        }
        else {
            std::cerr << "[Error] Script compilation failed for: " << scriptPath << " with exit code " << result << std::endl;
            return false;
        }
    }

    void LoadDll(const std::string path, const std::string fileName, Entity entity) {
        std::string fullPath = path + "/" + fileName + ".dll";
        std::cerr << "[Error] Failed to load DLL: " << fullPath << std::endl;
        HMODULE dll = LoadLibraryA(fullPath.c_str());
        if (!dll) {
            DWORD errorCode = GetLastError();
            std::cerr << "[Error] Failed to load DLL: " << fullPath << " Error code: " << errorCode << std::endl;
            return;
        }

        using CreateScriptFunc = Script * (*)();
        CreateScriptFunc createScript = (CreateScriptFunc)GetProcAddress(dll, "CreateScriptInstance");

        if (!createScript) {
            std::cerr << "[Error] Failed to find CreateScriptInstance in " << fileName << std::endl;
            return;
        }

        Script* rawScript = createScript();
        //rawScript->Init(entity);

        std::unique_ptr<Script> scriptInstance(rawScript);

        // Optional: Store the script instance somewhere
        std::cout << "[Info] Successfully created script instance from: " << fileName << std::endl;
    }

    void LoadScripts() {
        for (Entity entity : mEntities)
        {
            auto& script = coordinator->GetComponent<BehaviourScript>(entity);

            std::filesystem::path path(script.ScriptPath);
            std::string filename = path.stem().string();

            CompileScript(script.ScriptPath, "C:/Users/ragberr/Desktop/ExampleProj/Bin", EngineIncludePaths);

            LoadDll("C:/Users/ragberr/Desktop/ExampleProj/Bin", filename, entity);

        }
    }

private:

    std::shared_ptr<Coordinator> coordinator;
    std::unordered_map<std::string, std::unique_ptr<Script>> Scripts;
};