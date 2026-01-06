#pragma once
#include <cstdint>
#include <map>
#include <vector>
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
    Scene,          // .scene (or .json)
    Animation
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
    { ".json",      FileType::Scene },
    { ".reanim",    FileType::Animation },
    { ".rsm",       FileType::Animation }
};

inline static FileType GetFileType(const std::string& extension) {
    if (ExtensionMap.count(extension)) {
        return ExtensionMap.at(extension);
    }
    return FileType::Unknown;
}

inline static std::string GetDragPayloadType(FileType type) {
    switch (type) {
    case FileType::StaticMesh:   return "ASSET_STATIC_MESH";
    case FileType::SkeletalMesh: return "ASSET_SKELETAL_MESH";
    case FileType::Texture:      return "ASSET_TEXTURE";
    case FileType::Material:     return "ASSET_MATERIAL";
    case FileType::Scene:        return "ASSET_SCENE";
    case FileType::Code:         return "ASSET_CODE";
    case FileType::Animation:    return "ASSET_ANIMATION";
    default:                     return "ASSET_UNKNOWN";
    }
}

enum class AssetType : uint32_t {
    Null = -1,
    StaticMesh = 0,
    SkeletalMesh = 1,
    Texture = 2,
	Animation = 3
};

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

// --- NEW: Serialized Node Hierarchy ---
struct SerializedNode {
    std::string name;
    glm::mat4 transformation;
    std::vector<SerializedNode> children;
};

struct SerializedBoneAnim {
    std::string name;
    std::vector<std::pair<float, glm::vec3>> positions;
    std::vector<std::pair<float, glm::quat>> rotations;
    std::vector<std::pair<float, glm::vec3>> scales;
};

struct SerializedAnimation {
    std::string name;
    float duration;
    float ticksPerSecond;
    std::vector<SerializedBoneAnim> channels;

    // --- NEW: The Skeleton Hierarchy ---
    SerializedNode rootNode;
};