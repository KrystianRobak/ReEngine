#pragma once

#include <string>
#include <memory>
#include <vector>
#include <ReTypes.h>

// Forward Declarations
class CompiledMaterial;
class StaticMeshData;
class SkeletalMeshData;
struct TextureData;
struct TextureResource;

struct MeshResource {
    // Keep CPU data alive for context recovery or picking, 
    // but you could release it after upload if RAM is tight.
    std::shared_ptr<StaticMeshData> cpuMesh;

    std::atomic<bool> uploaded{ false };

    // GPU Handles (One per submesh)
    std::vector<uint32_t> VAOs;
    std::vector<uint32_t> VBOs;
    std::vector<uint32_t> EBOs;

    // Skeletal specific
    std::vector<uint32_t> BVAOs;

    // Cached counts for DrawElements
    std::vector<uint32_t> indexCounts;
};

using MeshResourceId = uint64_t;

class AssetManagerApi
{
public:
    virtual ~AssetManagerApi() = default;

    // --- Core Loop ---
    // Call this on the Render Thread to execute OpenGL commands
    virtual void DispatchUploads() = 0;

    // --- Mesh API ---
    // Returns a handle immediately. Check returned->uploaded to see if ready.
    // Handles automatic caching (FBX -> Custom Binary).
    virtual std::shared_ptr<MeshResource> GetMesh(const std::string& path) = 0;
    virtual std::shared_ptr<MeshResource> GetSkeletalMesh(const std::string& path) = 0;

    // --- Texture API ---
    virtual std::shared_ptr<TextureResource> GetTexture(const std::string& path) = 0;

    // --- Material API ---
    virtual void addMaterial(int id, CompiledMaterial material) = 0;
    virtual CompiledMaterial* GetMaterial(int id) = 0;

    // --- Utilities ---
    virtual int GetCurrentMaterialId() = 0;
    virtual int GetCurrentMeshId() = 0;
    virtual std::vector<std::string> GetCachedPaths() = 0;
    virtual std::vector<std::string> GetCachedTexturesPaths() = 0;
    virtual void unloadTexture(const std::string& path) = 0;
    virtual void unloadMesh(const std::string& path) = 0;
};