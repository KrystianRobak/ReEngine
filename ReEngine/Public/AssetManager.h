#pragma once

#include "Api/AssetManagerApi.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <string>
#include <future>
#include <memory>
#include <mutex>

#include "ThreadPool.h"
#include "StaticMesh.h"
#include "MeshData.h"
#include "StaticMeshData.h"
#include "TextureData.h"
#include "MaterialSystem/Material.h"
#include <SkeletalMeshComponent.h>



class ENGINE_API AssetManager : public AssetManagerApi {
public:
    AssetManager(ThreadPool* threadPool)
        : pool(threadPool) {
    }

    ~AssetManager() {
        shutdown();
    }

    MeshResourceId RegisterMesh(std::shared_ptr<StaticMeshData> cpuMesh) override;

    void UploadPendingResources() override;

    MeshResource* GetMeshResource(MeshResourceId id) override;

	std::vector<PendingStaticMesh>& GetPendingMeshes() { return pendingMeshes; }

    // Asynchronously load FBX asset
    std::future<std::shared_ptr<StaticMeshData>> loadFBX(const std::string& path) override;

	std::future<std::shared_ptr<TextureData>> loadTexture(const std::string& path) override;

	void unloadTexture(const std::string& path);

    TextureResource* GetTextureResource(const std::string& path) override 
    {
        std::lock_guard<std::mutex> lock(gpuTextureMutex);

        auto it = gpuTextures.find(path);
        if (it != gpuTextures.end())
            return it->second.get();

        return nullptr;
    }

    virtual std::vector<std::string> GetCachedTexturesPaths() override;

	void unloadMesh(const std::string& path);

	void addMaterial(int id, CompiledMaterial material);

    void AddPendingMesh(Entity entity, std::future<std::shared_ptr<StaticMeshData>> future) override;

    std::vector<std::string> GetCachedPaths();

    CompiledMaterial* GetMaterial(int id) override {
        auto it = materials.find(id);
        if (it != materials.end()) {
            return &it->second;
        }
        return nullptr;
	}

    virtual int GetCurrentMaterialId() override
    {
		return LastMaterialId++;
    }

    std::future<std::shared_ptr<SkeletalMeshData>> loadSkeletalFBX(const std::string& path) override;

    std::shared_ptr<SkeletalMeshData> importSkeletalMesh(const std::string& path);

    void processSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData& data, int meshIndex);

    void ExtractBoneWeightForVertices(std::vector<VertexBoneData>& vertices, aiMesh* mesh, const aiScene* scene, SkeletalMeshData& data);

    void shutdown();

private:
    ThreadPool* pool;
    std::unordered_map<std::string, std::weak_ptr<SkeletalMeshData>> SkeletalMeshCache;
	std::mutex skeletalCacheMutex;

    std::unordered_map<std::string, std::weak_ptr<StaticMeshData>> cache;

	std::unordered_map<std::string, std::weak_ptr<TextureData>> textureCache;

	std::unordered_map<int, CompiledMaterial> materials;

    std::unordered_map<MeshResourceId, std::shared_ptr<MeshResource>> meshResources;
    std::mutex meshResourcesMutex;
    MeshResourceId lastMeshResourceId = 0;

	int LastMaterialId = 0;

    std::unordered_map<std::string, std::shared_ptr<TextureResource>> gpuTextures;
    std::mutex gpuTextureMutex;

    std::mutex cacheMutex;
	std::mutex textureCacheMutex;

    std::vector<std::shared_ptr<TextureData>> pendingTextures;
	std::vector<PendingStaticMesh> pendingMeshes;

    std::shared_ptr<StaticMeshData> importFBX(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene, StaticMeshData& staticMesh);

    MeshData processMesh(aiMesh* mesh, const aiScene* scene);

    void loadTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<TextureData>& textures);
};