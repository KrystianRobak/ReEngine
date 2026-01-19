#pragma once

#include "ReEngineExport.h"
#include "Api/AssetManagerApi.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include <functional>
#include <atomic>
#include <filesystem>
#include <fstream>

#include "ThreadPool.h" 
#include "MeshData.h"
#include "StaticMeshData.h"
#include "SkeletalMeshData.h"
#include "TextureData.h"
#include "MaterialSystem/Material.h"
#include "AssetFileFormat.h" 
#include "AnimGraph.h"
#include "vaoutils.h"

// Forward declare
class Animation;
struct AssimpNodeData;

class ENGINE_API AssetManager : public AssetManagerApi {
public:
    AssetManager(ThreadPool* threadPool);
    ~AssetManager();

    // --- API Implementation ---
    void DispatchUploads() override;

    std::shared_ptr<MeshResource> GetMesh(const std::string& path) override;
    std::shared_ptr<MeshResource> GetSkeletalMesh(const std::string& path) override;
    std::shared_ptr<TextureResource> GetTexture(const std::string& path) override;

    std::shared_ptr<MeshResource> CreateManualMesh(const std::string& name, std::shared_ptr<StaticMeshData> data) override;
    void addMaterial(int id, CompiledMaterial material) override;
    CompiledMaterial* GetMaterial(int id) override;
    int GetCurrentMaterialId() override { return LastMaterialId++; }
    int GetCurrentMeshId() override { return lastMeshResourceId++; }

    void unloadTexture(const std::string& path) override;
    void unloadMesh(const std::string& path) override;

    std::shared_ptr<AnimationGraphResource> GetAnimationGraph(const std::string& path) override;
    std::shared_ptr<Animation> GetAnimation(const std::string& path, SkeletalMeshData* skeletalData) override;

    std::vector<std::string> GetCachedPaths() override;
    std::vector<std::string> GetCachedTexturesPaths() override;

    const std::unordered_map<int, CompiledMaterial>& GetMaterials() override{ return materials; }
    void LoadMaterial(int id, const std::string& path) override;

    void shutdown();

private:
    ThreadPool* pool;
    std::mutex assetMutex;
    std::mutex uploadMutex;

    std::queue<std::function<void()>> uploadQueue;

    std::unordered_map<std::string, std::shared_ptr<MeshResource>> meshCache;
    std::unordered_map<std::string, std::shared_ptr<TextureResource>> textureCache;
    std::unordered_map<int, CompiledMaterial> materials;
    std::unordered_map<std::string, std::shared_ptr<AnimationGraphResource>> graphCache;
    std::unordered_map<std::string, std::shared_ptr<Animation>> animationCache;

    std::atomic<int> lastMeshResourceId{ 0 };
    std::atomic<int> LastMaterialId{ 0 };
    std::atomic<int> lastTextureResourceId{ 0 };

    // --- Internal Helpers ---
    void EnqueueUpload(std::function<void()> func);

    // --- INTERNAL BINARY LOADERS ---
    std::shared_ptr<StaticMeshData> LoadBinaryStaticMesh(const std::string& path);
    std::shared_ptr<SkeletalMeshData> LoadBinarySkeletalMesh(const std::string& path);

    struct TextureLoadResult { int w, h, c; std::vector<unsigned char> pixels; };
    std::unique_ptr<TextureLoadResult> LoadBinaryTexture(const std::string& path);
    std::shared_ptr<Animation> LoadBinaryAnimation(const std::string& path);

    // --- TEMPLATE HELPERS ---
    template<typename T>
    void ReadVector(std::ifstream& in, std::vector<T>& vec) {
        uint32_t size = 0;
        in.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
        vec.resize(size);
        if (size > 0) in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
    }

    // Helper to read the specific MeshData structure (verts, normals, bones, etc.)
    void ReadMeshData(std::ifstream& in, MeshData& mesh);

    // Helper to read hierarchy
    void ReadSerializedNode(std::ifstream& in, AssimpNodeData& node);
};