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
#include "StaticMesh.h" // contains StaticMesh
#include "MeshData.h"   // contains Vertex, TextureData, MeshData
#include "StaticMeshData.h"
#include "TextureData.h"


class ENGINE_API AssetManager : public AssetManagerApi {
public:
    AssetManager(ThreadPool* threadPool)
        : pool(threadPool) {
    }

    ~AssetManager() {
        shutdown();
    }

	std::vector<PendingStaticMesh>& GetPendingMeshes() { return pendingMeshes; }

    // Asynchronously load FBX asset
    std::future<std::shared_ptr<StaticMeshData>> loadFBX(const std::string& path) override;

	std::future<std::shared_ptr<TextureData>> loadTexture(const std::string& path) override;

	void unloadTexture(const std::string& path);

	void unloadMesh(const std::string& path);


    void AddPendingMesh(Entity entity, std::future<std::shared_ptr<StaticMeshData>> future) override;

    std::vector<std::string> GetCachedPaths();

    void shutdown();

private:
    ThreadPool* pool;
    std::unordered_map<std::string, std::weak_ptr<StaticMeshData>> cache;

	std::unordered_map<std::string, std::weak_ptr<TextureData>> textureCache;

    std::mutex cacheMutex;
	std::mutex textureCacheMutex;

	std::vector<PendingStaticMesh> pendingMeshes;

    std::shared_ptr<StaticMeshData> importFBX(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene, StaticMeshData& staticMesh);

    MeshData processMesh(aiMesh* mesh, const aiScene* scene);

    void loadTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<TextureData>& textures);
};