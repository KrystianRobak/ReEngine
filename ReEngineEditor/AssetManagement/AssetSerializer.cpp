#include "AssetSerializer.h"


// --- STATIC MESH IMPLEMENTATION ---

bool AssetSerializer::SaveStaticMesh(const std::string& path, const StaticMeshData& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    // 1. Write Global Header
    AssetHeader header;
    header.type = AssetType::StaticMesh;
    header.dataSize = 0; // Can calculate if needed, or ignore for now
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    // 2. Write AABB
    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));

    // 3. Write Meshes
    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (const auto& mesh : data.meshes) {
        // Write raw Vertex data (Blazing fast because Vertex is a POD struct)
        WriteVector(out, mesh.vertices);

        // Write raw Index data
        WriteVector(out, mesh.indices);

        // Note: You can add TextureData paths here if needed, 
        // but typically you just save Material IDs and load materials separately.
    }

    out.close();
    return true;
}

std::shared_ptr<StaticMeshData> AssetSerializer::LoadStaticMesh(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    // 1. Read & Verify Header
    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC || header.type != AssetType::StaticMesh) {
        std::cerr << "Invalid asset file: " << path << std::endl;
        return nullptr;
    }

    auto result = std::make_shared<StaticMeshData>();
    result->path = path;

    // 2. Read AABB
    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));

    // 3. Read Meshes
    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    result->meshes.resize(meshCount);
    for (uint32_t i = 0; i < meshCount; ++i) {
        // Read directly into the vectors. 
        // No parsing, no loops, just memory copy.
        ReadVector(in, result->meshes[i].vertices);
        ReadVector(in, result->meshes[i].indices);
    }

    return result;
}

// --- SKELETAL MESH IMPLEMENTATION ---

bool AssetSerializer::SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    // 1. Header
    AssetHeader header;
    header.type = AssetType::SkeletalMesh;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    // 2. Standard Mesh Data (Same as Static)
    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));

    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (size_t i = 0; i < meshCount; i++) {
        WriteVector(out, data.meshes[i].vertices);
        WriteVector(out, data.meshes[i].indices);

        // 3. Write Bone Weights (Specific to Skeletal)
        // bonesPerMesh[i] corresponds to meshes[i]
        WriteVector(out, data.bonesPerMesh[i]);
    }

    // 4. Write Bone Info Map
    uint32_t boneCount = static_cast<uint32_t>(data.boneInfoMap.size());
    out.write(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

    for (const auto& [name, info] : data.boneInfoMap) {
        // Write String Name
        uint32_t nameLen = static_cast<uint32_t>(name.size());
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
        out.write(name.c_str(), nameLen);

        // Write Info Struct
        out.write(reinterpret_cast<const char*>(&info), sizeof(BoneInfo));
    }

    // (Optional: Write Animations here)

    return true;
}

std::shared_ptr<SkeletalMeshData> AssetSerializer::LoadSkeletalMesh(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC || header.type != AssetType::SkeletalMesh) return nullptr;

    auto result = std::make_shared<SkeletalMeshData>();
    result->path = path;

    // Read AABB
    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));

    // Read Meshes & Weights
    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));
    result->meshes.resize(meshCount);
    result->bonesPerMesh.resize(meshCount);

    for (uint32_t i = 0; i < meshCount; ++i) {
        ReadVector(in, result->meshes[i].vertices);
        ReadVector(in, result->meshes[i].indices);

        // Read Bone Weights directly into struct
        ReadVector(in, result->bonesPerMesh[i]);
    }

    // Read Bone Info Map
    uint32_t boneMapSize = 0;
    in.read(reinterpret_cast<char*>(&boneMapSize), sizeof(uint32_t));

    for (uint32_t i = 0; i < boneMapSize; ++i) {
        uint32_t nameLen;
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));

        std::string boneName;
        boneName.resize(nameLen);
        in.read(&boneName[0], nameLen);

        BoneInfo info;
        in.read(reinterpret_cast<char*>(&info), sizeof(BoneInfo));

        result->boneInfoMap[boneName] = info;
    }

    // Set bone count based on map size or stored value
    result->boneCount = boneMapSize;

    return result;
}

bool AssetSerializer::ImportFile(const std::string& path, const std::string& destination)
{
    return false;
}
