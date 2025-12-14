#pragma once
#include <cstdint>

// Unique signature to identify your files (e.g., "REAM" = RE Asset Mesh)
constexpr uint32_t ASSET_MAGIC = 0x4D414552;
constexpr uint32_t ASSET_VERSION = 1;

enum class AssetType : uint32_t {
    StaticMesh = 0,
    SkeletalMesh = 1,
    Texture = 2
};

// The very first bytes of your file
struct AssetHeader {
    uint32_t magic = ASSET_MAGIC;
    uint32_t version = ASSET_VERSION;
    AssetType type;
    uint32_t dataSize; // Total size of the data blob following header
};

// Header for a specific sub-mesh (corresponds to one 'MeshData' entry)
struct MeshSectionHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t materialIndex; // If you track materials by index
    // We don't store pointers here, only counts!
};