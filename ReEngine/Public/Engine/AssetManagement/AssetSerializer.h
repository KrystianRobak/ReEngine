#pragma once
#include <string>
#include <memory>
#include "StaticMeshData.h"
#include "AssetFileFormat.h"

class AssetSerializer {
public:
    // --- COOKING (Save to disk) ---
    // Takes your runtime structures and dumps them as binary
    static bool SaveStaticMesh(const std::string& path, const StaticMeshData& data);
    static bool SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data);

    // --- LOADING (Read from disk) ---
    // Reads binary directly into memory (Extremely Fast)
    static std::shared_ptr<StaticMeshData> LoadStaticMesh(const std::string& path);
    static std::shared_ptr<SkeletalMeshData> LoadSkeletalMesh(const std::string& path);
};