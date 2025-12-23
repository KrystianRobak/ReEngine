#pragma once
#include <cstdint>

// Unique signature to identify your files (e.g., "REAM" = RE Asset Mesh)
constexpr uint32_t ASSET_MAGIC = 0x4D414552;
constexpr uint32_t ASSET_VERSION = 1;

enum class FileType {
    Folder,
    Unknown,
    Code,           // .cpp, .h
    StaticMesh,     // .remesh
    SkeletalMesh,   // .reskel
    Texture,        // .retex
    Material,       // .material
    Scene           // .scene (or .json)
};

static const std::map<std::string, FileType> ExtensionMap = {
    { ".cpp",       FileType::Code },
    { ".h",         FileType::Code },
    { ".hpp",       FileType::Code },
    { ".remesh",    FileType::StaticMesh },   // Your custom static mesh
    { ".reskel",    FileType::SkeletalMesh }, // Your custom skeletal mesh
    { ".retex",     FileType::Texture },      // Your custom texture
    { ".material",  FileType::Material },
    { ".scene",     FileType::Scene },        // Renamed .json to .scene for clarity?
    { ".json",      FileType::Scene }
};

constexpr inline static FileType GetFileType(const std::string& extension) {
    if (ExtensionMap.count(extension)) {
        return ExtensionMap.at(extension);
    }
    return FileType::Unknown;
}

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

struct TextureHeader {
    uint32_t width;
    uint32_t height;
    uint32_t channels; // usually 4 for RGBA
    uint32_t dataSize; // bytes of pixel data
};

// Header for a specific sub-mesh (corresponds to one 'MeshData' entry)
struct MeshSectionHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t materialIndex; // If you track materials by index
    // We don't store pointers here, only counts!
};