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

static glm::quat QuatFromAssimp(const aiQuaternion& from) {
    return glm::quat(from.w, from.x, from.y, from.z);
}

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

std::pair<AssetType, std::string>  AssetSerializer::ImportAndCookFile(const std::string& sourcePath, const std::string& destDir) {
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

            importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

            // Preserving hierarchy is often useful for skeletal meshes
            const aiScene* scene = importer.ReadFile(sourcePath,
                aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_LimitBoneWeights);

            if (!scene || !scene->mRootNode) {
                std::cerr << "[Importer] Assimp Error: " << importer.GetErrorString() << std::endl;
                return { AssetType::Null, "" };
            }

            // --- NEW: ANIMATION EXPORT SECTION ---
            if (scene->HasAnimations()) {
                std::cout << "[Importer] Found " << scene->mNumAnimations << " animations." << std::endl;

                for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                    aiAnimation* anim = scene->mAnimations[i];

                    // Determine a filename for the animation
                    std::string animName = anim->mName.C_Str();
                    if (animName.empty()) animName = "Anim_" + std::to_string(i);

                    // Clean invalid characters from filename if necessary
                    std::replace(animName.begin(), animName.end(), ':', '_');
                    std::replace(animName.begin(), animName.end(), '|', '_');

                    std::string outFileName = filename + "_" + animName + ".reanim";
                    std::string fullPath = (destFolder / outFileName).string();

                    // Process and Save
                    SerializedAnimation animData = ProcessAnimation(anim, scene);
                    if (SaveAnimation(fullPath, animData)) {
                        std::cout << "[Importer] Saved Animation: " << outFileName << std::endl;
                    }
                }
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
                    SaveSkeletalMesh(outPath, *skelData);
                    return { AssetType::SkeletalMesh, outPath };
                }
            }
            else {
                // Cook as STATIC MESH (.remesh)
                std::cout << "[Importer] No Bones. Cooking as Static Mesh..." << std::endl;
                auto staticData = ImportStaticMeshAssimp(sourcePath);
                if (staticData) {
                    std::string outPath = (destFolder / (filename + ".remesh")).string();
                    SaveStaticMesh(outPath, *staticData);
                    return {AssetType::StaticMesh, outPath };
                }
            }
        }
        catch (std::exception& e) {
            std::cerr << "[Importer] Failed: " << sourcePath << " -> " << e.what() << std::endl;
            return { AssetType::Null, "" };
        }
    }
    // --- TEXTURES ---
    else if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") {
        std::string outPath = (destFolder / (filename + ".retex")).string();
        bool result = ImportTexture(sourcePath, outPath);

        if (result)
        {

            return { AssetType::Texture, outPath };
        }
        else
        {
            return {AssetType::Null,""};
        } 
    }

    return { AssetType::Null, "" };
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

void AssetSerializer::ConvertAssimpNode(const aiNode* src, SerializedNode& dst) {
    dst.name = src->mName.C_Str();
    dst.transformation = Mat4FromAssimp(src->mTransformation);

    for (unsigned int i = 0; i < src->mNumChildren; i++) {
        SerializedNode child;
        ConvertAssimpNode(src->mChildren[i], child);
        dst.children.push_back(child);
    }
}

SerializedAnimation AssetSerializer::ProcessAnimation(const aiAnimation* anim, const aiScene* scene) {
    SerializedAnimation outAnim;
    outAnim.name = anim->mName.C_Str();
    outAnim.duration = (float)anim->mDuration;
    outAnim.ticksPerSecond = (anim->mTicksPerSecond != 0) ? (float)anim->mTicksPerSecond : 25.0f;

    // 1. Process Channels (Existing code)
    for (unsigned int i = 0; i < anim->mNumChannels; i++) {
        aiNodeAnim* channel = anim->mChannels[i];
        SerializedBoneAnim boneAnim;
        boneAnim.name = channel->mNodeName.C_Str();

        for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
            boneAnim.positions.push_back({ (float)channel->mPositionKeys[k].mTime, Vec3FromAssimp(channel->mPositionKeys[k].mValue) });
        }
        for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
            boneAnim.rotations.push_back({ (float)channel->mRotationKeys[k].mTime, QuatFromAssimp(channel->mRotationKeys[k].mValue) });
        }
        for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
            boneAnim.scales.push_back({ (float)channel->mScalingKeys[k].mTime, Vec3FromAssimp(channel->mScalingKeys[k].mValue) });
        }
        outAnim.channels.push_back(boneAnim);
    }

    // 2. Process Hierarchy (NEW)
    ConvertAssimpNode(scene->mRootNode, outAnim.rootNode);

    return outAnim;
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

// --- NEW HELPER: Recursive Node Write ---
void WriteSerializedNode(std::ofstream& out, const SerializedNode& node) {
    // Name
    uint32_t nameLen = (uint32_t)node.name.size();
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) out.write(node.name.c_str(), nameLen);

    // Transform
    out.write(reinterpret_cast<const char*>(&node.transformation), sizeof(glm::mat4));

    // Children
    uint32_t childCount = (uint32_t)node.children.size();
    out.write(reinterpret_cast<const char*>(&childCount), sizeof(uint32_t));

    for (const auto& child : node.children) {
        WriteSerializedNode(out, child);
    }
}
bool AssetSerializer::SaveAnimation(const std::string& path, const SerializedAnimation& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    AssetHeader header;
    header.magic = ASSET_MAGIC;
    header.type = (AssetType)3; // Animation
    header.version = 1;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    // Metadata
    out.write(reinterpret_cast<const char*>(&data.duration), sizeof(float));
    out.write(reinterpret_cast<const char*>(&data.ticksPerSecond), sizeof(float));

    // Animation Name
    uint32_t nameLen = (uint32_t)data.name.size();
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) out.write(data.name.c_str(), nameLen);

    // Channels
    uint32_t numChannels = (uint32_t)data.channels.size();
    out.write(reinterpret_cast<const char*>(&numChannels), sizeof(uint32_t));

    for (const auto& channel : data.channels) {
        // Channel Name
        uint32_t bLen = (uint32_t)channel.name.size();
        out.write(reinterpret_cast<const char*>(&bLen), sizeof(uint32_t));
        if (bLen > 0) out.write(channel.name.c_str(), bLen);

        // --- FIXED: Write Loop (Prevents Padding Corruption) ---

        // 1. Positions
        uint32_t nPos = (uint32_t)channel.positions.size();
        out.write(reinterpret_cast<const char*>(&nPos), sizeof(uint32_t));
        for (const auto& val : channel.positions) {
            out.write(reinterpret_cast<const char*>(&val.first), sizeof(float));      // Time
            out.write(reinterpret_cast<const char*>(&val.second), sizeof(glm::vec3)); // Value
        }

        // 2. Rotations
        uint32_t nRot = (uint32_t)channel.rotations.size();
        out.write(reinterpret_cast<const char*>(&nRot), sizeof(uint32_t));
        for (const auto& val : channel.rotations) {
            out.write(reinterpret_cast<const char*>(&val.first), sizeof(float));
            out.write(reinterpret_cast<const char*>(&val.second), sizeof(glm::quat));
        }

        // 3. Scales
        uint32_t nScl = (uint32_t)channel.scales.size();
        out.write(reinterpret_cast<const char*>(&nScl), sizeof(uint32_t));
        for (const auto& val : channel.scales) {
            out.write(reinterpret_cast<const char*>(&val.first), sizeof(float));
            out.write(reinterpret_cast<const char*>(&val.second), sizeof(glm::vec3));
        }
    }

    // Hierarchy
    WriteSerializedNode(out, data.rootNode);

    return true;
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