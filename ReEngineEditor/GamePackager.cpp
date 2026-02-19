#include "GamePackager.h"
#include "Logger.h"
#include <fstream>
#include <iostream>
#include <windows.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

void GamePackager::PackageGame(const std::string& buildName, const std::string& destinationPath) {
    fs::path destDir = fs::path(destinationPath) / buildName;

    try {

        if (fs::exists(destDir)) fs::remove_all(destDir);
        fs::create_directories(destDir);
        fs::create_directories(destDir / "Systems");

        fs::path sourceConfigPath = "";
        json projectConfig;

        for (const auto& entry : fs::directory_iterator(fs::current_path())) {
            std::string filename = entry.path().filename().string();
            if (filename.find("hotreload") != std::string::npos) continue;

            if (entry.path().extension() == ".json") {
                std::ifstream f(entry.path());
                try {
                    json temp;
                    f >> temp;
                    if (temp.contains("engine_path")) {
                        sourceConfigPath = entry.path();
                        projectConfig = temp;
                        break;
                    }
                }
                catch (...) { continue; }
            }
        }

        if (sourceConfigPath.empty()) {
            LOGF_ERROR("Could not find project config (e.g., stopa.json)!");
            return;
        }

        LOGF_INFO("Found Config: %s", sourceConfigPath.string().c_str());

        std::string engineExePath = projectConfig["engine_path"];
        fs::path engineBinDir = fs::path(engineExePath).parent_path();

        if (!fs::exists(engineBinDir)) {
            LOGF_ERROR("Engine path from config does not exist: %s", engineBinDir.string().c_str());
            return;
        }

        CopyBinaries(destDir, engineBinDir, projectConfig);

        if (projectConfig.contains("name")) {
            std::string gameName = projectConfig["name"];
            std::string gameDllName = gameName + ".dll";
            std::string gamePdbName = gameName + ".pdb";

            fs::path projectBinDir = fs::current_path() / "bin" / "Debug";
            fs::path gameDllSrc = projectBinDir / gameDllName;
            fs::path gameDllDest = destDir / gameDllName;

            if (fs::exists(gameDllSrc)) {
                fs::copy_file(gameDllSrc, gameDllDest, fs::copy_options::overwrite_existing);
                LOGF_INFO("Copied Game DLL: %s", gameDllName.c_str());

                fs::path pdbSrc = projectBinDir / gamePdbName;
                if (fs::exists(pdbSrc)) {
                    fs::copy_file(pdbSrc, destDir / gamePdbName, fs::copy_options::overwrite_existing);
                }
            }
            else {
                LOGF_ERROR("Could not find Game DLL at: %s", gameDllSrc.string().c_str());
                if (fs::exists(fs::current_path() / gameDllName)) {
                    fs::copy_file(fs::current_path() / gameDllName, gameDllDest, fs::copy_options::overwrite_existing);
                    LOGF_WARN("Found Game DLL in root folder instead of bin/Debug.");
                }
            }
        }

        fs::path destContent = destDir / "Content";
        CopyContent(destContent, "Content");

        destContent = destDir / "shaders";
        CopyContent(destContent, "shaders");

        fs::path destConfigPath = destDir / sourceConfigPath.filename();
        fs::copy_file(sourceConfigPath, destConfigPath, fs::copy_options::overwrite_existing);
        SanitizeSingleFile(destConfigPath);

        std::ofstream ini(destDir / "Game.ini");
        ini << "Config=" << sourceConfigPath.filename().string() << "\n";

        std::string startScene = "Content/Scenes/AutoSaveScene.json";

        if (projectConfig.contains("EntryScene")) {
            if (projectConfig["EntryScene"].is_string()) {
                std::string entryVal = projectConfig["EntryScene"].get<std::string>();

                if (!entryVal.empty()) {
                    std::replace(entryVal.begin(), entryVal.end(), '\\', '/');
                    size_t pos = entryVal.find("Content/");
                    if (pos != std::string::npos) {
                        startScene = entryVal.substr(pos);
                    }
                    else {
                        startScene = entryVal;
                    }
                }
            }
        }

        ini << "StartScene=" << "Content/Scenes/" + startScene;
        ini.close();

        LOGF_INFO("Packaging Complete! Start Scene: %s", startScene.c_str());
        ShellExecuteA(NULL, "open", destDir.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);

        LOGF_INFO("Packaging Complete! Output: %s", destDir.string().c_str());
        ShellExecuteA(NULL, "open", destDir.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }
    catch (const std::exception& e) {
        LOGF_ERROR("Packaging Failed: %s", e.what());
    }
}

void GamePackager::CopyBinaries(const fs::path& dest, const fs::path& engineBinDir, const json& config) {
    fs::path systemsSourceDir = engineBinDir / "Systems";
    fs::path systemsDestDir = dest / "Systems";

    LOGF_INFO("Copying binaries from: %s", engineBinDir.string().c_str());

    std::vector<std::string> coreBinaries = {
        "ReEngine.dll",
        "ReEngineCore.dll",
        "ReflectionCore.dll",
        "assimp-vc143-mtd.dll",
        "freeglutd.dll",
        "glew32.dll",
        "glfw3.dll",
        "PhysX_64.dll",
        "PhysXCommon_64.dll",
        "PhysXCooking_64.dll",
        "PhysXFoundation_64.dll",
        "PVDRuntime_64.dll"
    };

    for (const auto& binary : coreBinaries) {
        fs::path src = engineBinDir / binary;
        fs::path target = dest / binary;

        if (fs::exists(src)) {
            fs::copy_file(src, target, fs::copy_options::overwrite_existing);
        }
        else {
            LOGF_WARN("Missing Core Binary: %s", binary.c_str());
        }
    }

    fs::path runnerSrc = engineBinDir / "ReEngineRunner.exe";

    std::string gameName = "Game";
    if (config.contains("name")) gameName = config["name"];
    fs::path runnerDest = dest / (gameName + ".exe");

    if (fs::exists(runnerSrc)) {
        fs::copy_file(runnerSrc, runnerDest, fs::copy_options::overwrite_existing);
        LOGF_INFO("Copied Runner to: %s", runnerDest.string().c_str());
    }
    else {
        LOGF_ERROR("ReEngineRunner.exe not found at %s. Please build the Runner project!", runnerSrc.string().c_str());
    }

    auto CopyModule = [&](const std::string& moduleName) {
        std::string dllName = moduleName + ".dll";
        fs::path src = systemsSourceDir / dllName;
        fs::path target = systemsDestDir / dllName;

        if (fs::exists(src)) {
            fs::copy_file(src, target, fs::copy_options::overwrite_existing);
        }
        else {
            LOGF_WARN("Missing System Module: %s", src.string().c_str());
        }
        };

    if (config.contains("Renderer")) CopyModule(config["Renderer"]);
    if (config.contains("Physics")) CopyModule(config["Physics"]);
    if (config.contains("ModulesToLoad")) {
        for (const auto& mod : config["ModulesToLoad"]) {
            CopyModule(mod);
        }
    }
}

void GamePackager::CopyContent(const fs::path& dest, std::string sourceFolderName) {
    fs::path src = fs::current_path() / sourceFolderName;

    if (fs::exists(src)) {
        LOGF_INFO("Copying Shaders from: %s", src.string().c_str());
        try {
            fs::create_directories(dest);
            fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }
        catch (std::filesystem::filesystem_error& e) {
            LOGF_ERROR("Failed to copy shaders: %s", e.what());
        }
    }
    else {
        LOGF_WARN("Shaders folder not found at %s. (Skipping)", src.string().c_str());
    }
}


void GamePackager::SanitizeJson(json& j) {
    if (j.is_string()) {
        std::string val = j.get<std::string>();
        if (val.find("Content") != std::string::npos &&
            (val.find("/") != std::string::npos || val.find("\\") != std::string::npos)) {
            size_t pos = val.find("Content");
            if (pos != std::string::npos) {
                std::string relativePath = "./" + val.substr(pos);
                std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
                j = relativePath;
            }
        }
    }
    else if (j.is_array()) {
        for (auto& element : j) SanitizeJson(element);
    }
    else if (j.is_object()) {
        for (auto& [key, value] : j.items()) SanitizeJson(value);
    }
}

void GamePackager::SanitizeSingleFile(const fs::path& filePath) {
    std::ifstream inFile(filePath);
    json j;
    try { inFile >> j; }
    catch (...) { return; }
    inFile.close();
    SanitizeJson(j);
    std::ofstream outFile(filePath);
    outFile << j.dump(4);
    outFile.close();
}

void GamePackager::SanitizeProjectFiles(const fs::path& destContentDir) {
    if (!fs::exists(destContentDir)) return;
    for (const auto& entry : fs::recursive_directory_iterator(destContentDir)) {
        if (entry.path().extension() == ".json" || entry.path().extension() == ".scene") {
            SanitizeSingleFile(entry.path());
        }
    }
}