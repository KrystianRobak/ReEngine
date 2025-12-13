#pragma once
#include <future>
#include <memory>
#include <string>
#include <ReTypes.h>
//#include "MaterialSystem/Material.h"

class CompiledMaterial;
class StaticMeshData;
class SkeletalMeshData;
class PendingStaticMesh;
struct TextureData;
struct TextureResource;


struct MeshResource {
	std::shared_ptr<StaticMeshData> cpuMesh; // CPU-side arrays, valid until uploaded (shared_ptr)
	std::atomic<bool> needsUpload{ true };  // set by game thread
	std::atomic<bool> uploaded{ false };    // set by render thread

	// GL handles (only ever touched by render thread)
	uint32_t VAO = 0;
	uint32_t VBO = 0;
	uint32_t EBO = 0;
	uint32_t BVAO = 0;
	uint32_t indexCount = 0;
};
using MeshResourceId = uint64_t;

class AssetManagerApi
{
public:

	virtual std::future<std::shared_ptr<StaticMeshData>> loadFBX(const std::string& path) = 0;

	virtual std::future<std::shared_ptr<SkeletalMeshData>> loadSkeletalFBX(const std::string& path) = 0;

	virtual MeshResourceId RegisterMesh(std::shared_ptr<StaticMeshData> cpuMesh) = 0;;

	virtual void UploadPendingResources() = 0;

	virtual MeshResource* GetMeshResource(MeshResourceId id) = 0;

	virtual CompiledMaterial* GetMaterial(int id) = 0;

	virtual void AddPendingMesh(Entity entity, std::future<std::shared_ptr<StaticMeshData>> future) = 0;

	virtual std::future<std::shared_ptr<TextureData>> loadTexture(const std::string& path) = 0;

	virtual TextureResource* GetTextureResource(const std::string& path) = 0;

	virtual int GetCurrentMaterialId() = 0;

	virtual std::vector<std::string> GetCachedPaths() = 0;

	virtual std::vector<std::string> GetCachedTexturesPaths() = 0;

	virtual void unloadTexture(const std::string& path) = 0;

	virtual void unloadMesh(const std::string& path) = 0;

	virtual void addMaterial(int id, CompiledMaterial material) = 0;

	virtual std::vector<PendingStaticMesh>& GetPendingMeshes() = 0;
};