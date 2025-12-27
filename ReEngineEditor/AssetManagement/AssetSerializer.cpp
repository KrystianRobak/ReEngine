#include "AssetSerializer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <map>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// GLM (Ensure you have GLM included)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace fs = std::filesystem;

// --- HELPERS ---

static glm::mat4 Mat4FromAssimp(const aiMatrix4x4& from) {
    glm::mat4 to;
    // Assimp is Row Major, GLM is Column Major
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static glm::vec3 Vec3FromAssimp(const aiVector3D& vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

// --- FILE I/O HELPERS ---

template<typename T>
void WriteVector(std::ofstream& out, const std::vector<T>& vec) {
    uint32_t size = static_cast<uint32_t>(vec.size());
    out.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    if (size > 0) {
        out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
    }
}

template<typename T>
void ReadVector(std::ifstream& in, std::vector<T>& vec) {
    uint32_t size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    vec.resize(size);
    if (size > 0) {
        in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
    }
}

// --- MAIN IMPORT LOGIC ---

bool AssetSerializer::ImportAndCookFile(const std::string& sourcePath, const std::string& destDir) {
    fs::path src(sourcePath);
    std::string ext = src.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    std::string filename = src.stem().string();
    fs::path destFolder(destDir);

    // --- 3D MODELS ---
    if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb") {
        try {
            // 1. Peek at the file using Assimp to decide if it's Static or Skeletal
            Assimp::Importer importer;
            // Preserving hierarchy is often useful for skeletal meshes
            const aiScene* scene = importer.ReadFile(sourcePath,
                aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_LimitBoneWeights);

            if (!scene || !scene->mRootNode) {
                std::cerr << "[Importer] Assimp Error: " << importer.GetErrorString() << std::endl;
                return false;
            }

            // Heuristic: Check if any mesh has bones
            bool hasBones = false;
            for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                if (scene->mMeshes[i]->HasBones()) {
                    hasBones = true;
                    break;
                }
            }

            if (hasBones) {
                // Cook as SKELETAL MESH (.reskel)
                std::cout << "[Importer] Detected Bones. Cooking as Skeletal Mesh..." << std::endl;
                auto skelData = ImportSkeletalMeshAssimp(sourcePath);
                if (skelData) {
                    std::string outPath = (destFolder / (filename + ".reskel")).string();
                    return SaveSkeletalMesh(outPath, *skelData);
                }
            }
            else {
                // Cook as STATIC MESH (.remesh)
                std::cout << "[Importer] No Bones. Cooking as Static Mesh..." << std::endl;
                auto staticData = ImportStaticMeshAssimp(sourcePath);
                if (staticData) {
                    std::string outPath = (destFolder / (filename + ".remesh")).string();
                    return SaveStaticMesh(outPath, *staticData);
                }
            }
        }
        catch (std::exception& e) {
            std::cerr << "[Importer] Failed: " << sourcePath << " -> " << e.what() << std::endl;
            return false;
        }
    }
    // --- TEXTURES ---
    else if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") {
        std::string outPath = (destFolder / (filename + ".retex")).string();
        return ImportTexture(sourcePath, outPath);
    }

    return false;
}

// --- STATIC MESH IMPLEMENTATION ---

MeshData AssetSerializer::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    MeshData myMesh;

    // 1. Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Position
        vertex.Position = Vec3FromAssimp(mesh->mVertices[i]);

        // Normal
        if (mesh->HasNormals())
            vertex.Normal = Vec3FromAssimp(mesh->mNormals[i]);
        else
            vertex.Normal = glm::vec3(0.0f);

        // TexCoords
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f);
        }

        // (Optional: Tangents/Bitangents if your Vertex struct has them)

        myMesh.vertices.push_back(vertex);
    }

    // 2. Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            myMesh.indices.push_back(face.mIndices[j]);
        }
    }

    return myMesh;
}

void AssetSerializer::ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData& outData)
{
}

std::shared_ptr<StaticMeshData> AssetSerializer::ImportStaticMeshAssimp(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode) return nullptr;

    auto data = std::make_shared<StaticMeshData>();
    data->path = path;

    // Helper to calculate AABB
    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        MeshData processedMesh = ProcessMesh(mesh, scene);

        // Update AABB
        for (const auto& v : processedMesh.vertices) {
            min = glm::min(min, v.Position);
            max = glm::max(max, v.Position);
        }

        data->meshes.push_back(processedMesh);
    }

    data->aabbMin = min;
    data->aabbMax = max;

    return data;
}

bool AssetSerializer::SaveStaticMesh(const std::string& path, const StaticMeshData& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    AssetHeader header;
    header.type = AssetType::StaticMesh;
    header.dataSize = 0; // Optional to calculate
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));

    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (const auto& mesh : data.meshes) {
        WriteVector(out, mesh.vertices);
        WriteVector(out, mesh.indices);
    }

    return true;
}

std::shared_ptr<StaticMeshData> AssetSerializer::LoadStaticMesh(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC || header.type != AssetType::StaticMesh) return nullptr;

    auto result = std::make_shared<StaticMeshData>();
    result->path = path;

    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));

    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    result->meshes.resize(meshCount);
    for (uint32_t i = 0; i < meshCount; ++i) {
        ReadVector(in, result->meshes[i].vertices);
        ReadVector(in, result->meshes[i].indices);
    }

    return result;
}

// --- SKELETAL MESH IMPLEMENTATION ---

std::shared_ptr<SkeletalMeshData> AssetSerializer::ImportSkeletalMeshAssimp(const std::string& path) {
    Assimp::Importer importer;
    // NOTE: Do not use OptimizeMeshes for Skeletal meshes immediately if it breaks bone logic, 
    // but usually it's fine. LimitBoneWeights is crucial for Game Engines (usually max 4).
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_LimitBoneWeights);

    if (!scene || !scene->mRootNode) return nullptr;

    auto data = std::make_shared<SkeletalMeshData>();
    data->path = path;

    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);

    // Process all meshes
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMeshPtr = scene->mMeshes[i];

        // 1. Process Geometry (Vertices/Indices) same as Static
        MeshData meshData = ProcessMesh(aiMeshPtr, scene);

        // Update AABB
        for (const auto& v : meshData.vertices) {
            min = glm::min(min, v.Position);
            max = glm::max(max, v.Position);
        }

        data->meshes.push_back(meshData);

        // 2. Process Bones
        std::vector<VertexBoneData> meshBoneData;
        meshBoneData.resize(meshData.vertices.size()); // Resize to match vertex count

        // Iterate over bones in this mesh
        for (unsigned int boneIndex = 0; boneIndex < aiMeshPtr->mNumBones; ++boneIndex) {
            aiBone* bone = aiMeshPtr->mBones[boneIndex];
            std::string boneName = bone->mName.C_Str();
            int boneID = -1;

            // Check if bone already exists in our global map
            if (data->boneInfoMap.find(boneName) == data->boneInfoMap.end()) {
                BoneInfo newInfo;
                newInfo.id = data->boneCount;
                newInfo.offset = Mat4FromAssimp(bone->mOffsetMatrix);
                data->boneInfoMap[boneName] = newInfo;
                boneID = data->boneCount;
                data->boneCount++;
            }
            else {
                boneID = data->boneInfoMap[boneName].id;
            }

            // Assign weights to vertices
            // weights look like: (vertexId, weightValue)
            for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
                aiVertexWeight weight = bone->mWeights[w];
                if (weight.mVertexId < meshBoneData.size()) {
                    meshBoneData[weight.mVertexId].AddBoneData(boneID, weight.mWeight);
                }
            }
        }

        data->bonesPerMesh.push_back(meshBoneData);
    }

    data->aabbMin = min;
    data->aabbMax = max;

    return data;
}

bool AssetSerializer::SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    AssetHeader header;
    header.type = AssetType::SkeletalMesh;
    header.dataSize = 0;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    // AABB
    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));

    // Meshes Count
    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (size_t i = 0; i < meshCount; i++) {
        // Write Geometry
        WriteVector(out, data.meshes[i].vertices);
        WriteVector(out, data.meshes[i].indices);

        // Write Bone Data (Specific to Skeletal)
        // Note: VertexBoneData is a POD struct, so WriteVector works safely
        WriteVector(out, data.bonesPerMesh[i]);
    }

    // Write Bone Info Map
    uint32_t boneCount = static_cast<uint32_t>(data.boneInfoMap.size());
    out.write(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

    for (const auto& [name, info] : data.boneInfoMap) {
        uint32_t nameLen = static_cast<uint32_t>(name.size());
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
        out.write(name.c_str(), nameLen);
        out.write(reinterpret_cast<const char*>(&info), sizeof(BoneInfo));
    }

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

    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));

    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    result->meshes.resize(meshCount);
    result->bonesPerMesh.resize(meshCount);

    for (uint32_t i = 0; i < meshCount; ++i) {
        ReadVector(in, result->meshes[i].vertices);
        ReadVector(in, result->meshes[i].indices);
        ReadVector(in, result->bonesPerMesh[i]);
    }

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
    result->boneCount = boneMapSize;

    return result;
}

// --- TEXTURE IMPLEMENTATION ---

bool AssetSerializer::ImportTexture(const std::string& source, const std::string& dest) {
    int w, h, c;
    // stbi_load allocates memory, we must free it
    unsigned char* data = stbi_load(source.c_str(), &w, &h, &c, 4); // Force 4 channels (RGBA)
    if (!data) {
        std::cerr << "Failed to load texture: " << source << std::endl;
        return false;
    }

    std::ofstream out(dest, std::ios::binary);
    if (!out.is_open()) {
        stbi_image_free(data);
        return false;
    }

    // 1. Asset Header
    AssetHeader header;
    header.magic = ASSET_MAGIC;
    header.version = ASSET_VERSION;
    header.type = AssetType::Texture;

    // 2. Texture Header
    TextureHeader texHeader;
    texHeader.width = static_cast<uint32_t>(w);
    texHeader.height = static_cast<uint32_t>(h);
    texHeader.channels = 4;
    texHeader.dataSize = texHeader.width * texHeader.height * texHeader.channels;

    header.dataSize = sizeof(TextureHeader) + texHeader.dataSize;

    // Write all
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    out.write(reinterpret_cast<char*>(&texHeader), sizeof(TextureHeader));
    out.write(reinterpret_cast<char*>(data), texHeader.dataSize);

    out.close();
    stbi_image_free(data);
    return true;
}