#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "json/json.hpp"

class GamePackager {
public:
    static void PackageGame(const std::string& buildName, const std::string& destinationPath);

private:
    static void CopyBinaries(const std::filesystem::path& dest, const std::filesystem::path& engineBinDir, const nlohmann::json& config);
    static void CopyContent(const std::filesystem::path& dest, std::string sourceFolderName);

    static void SanitizeProjectFiles(const std::filesystem::path& destContentDir);
    static void SanitizeSingleFile(const std::filesystem::path& filePath);
    static void SanitizeJson(nlohmann::json& j);
};