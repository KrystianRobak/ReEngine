#pragma once
#include <string>
#include <memory>
#include <vector>
#include "ReEngineExport.h"
#include "StaticMeshData.h"
#include "SkeletalMeshData.h"
#include "AssetFileFormat.h"

// Forward declarations
struct aiScene;
struct aiMesh;
struct aiNode;

class ENGINE_API AssetSerializer {
public:
    // --- COOKING (Save to disk) ---
    static bool SaveStaticMesh(const std::string& path, const StaticMeshData& data);
    static std::shared_ptr<StaticMeshData> LoadStaticMesh(const std::string& path);

    static bool SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data);
    static std::shared_ptr<SkeletalMeshData> LoadSkeletalMesh(const std::string& path);

    static bool SaveAnimation(const std::string& path, const SerializedAnimation& data);

    // Main Entry point for dragging and dropping files
    // Returns true if successfully cooked into a .remesh, .reskel, or .retex
    static std::pair<AssetType,std::string> ImportAndCookFile(const std::string& sourcePath, const std::string& destDir);

    static bool ImportTexture(const std::string& source, const std::string& dest);

private:
    // Internal Importers
    static std::shared_ptr<StaticMeshData> ImportStaticMeshAssimp(const std::string& path);
    static std::shared_ptr<SkeletalMeshData> ImportSkeletalMeshAssimp(const std::string& path);

    // Helpers
    static MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
    static void ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData& outData);
    static SerializedAnimation ProcessAnimation(const struct aiAnimation* anim);
};