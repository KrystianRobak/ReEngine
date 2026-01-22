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

void WriteMeshData(std::ofstream& out, const MeshData& mesh) {
    // 1. Scalar Data
    out.write(reinterpret_cast<const char*>(&mesh.materialIndex), sizeof(int));
    out.write(reinterpret_cast<const char*>(&mesh.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&mesh.aabbMax), sizeof(glm::vec3));

    // 2. Vector Data
    WriteVector(out, mesh.vertices);
    WriteVector(out, mesh.Normals);
    WriteVector(out, mesh.TexCoords);
    WriteVector(out, mesh.Tangents);
    WriteVector(out, mesh.Bitangents);

    // 3. Animation Data (Write even if empty, WriteVector handles size=0 gracefully)
    WriteVector(out, mesh.boneIDs);
    WriteVector(out, mesh.weights);

    // 4. Indices
    WriteVector(out, mesh.indices);
}

void ReadMeshData(std::ifstream& in, MeshData& mesh) {
    // 1. Scalar Data
    in.read(reinterpret_cast<char*>(&mesh.materialIndex), sizeof(int));
    in.read(reinterpret_cast<char*>(&mesh.aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&mesh.aabbMax), sizeof(glm::vec3));

    // 2. Vector Data
    ReadVector(in, mesh.vertices);
    ReadVector(in, mesh.Normals);
    ReadVector(in, mesh.TexCoords);
    ReadVector(in, mesh.Tangents);
    ReadVector(in, mesh.Bitangents);

    // 3. Animation Data
    ReadVector(in, mesh.boneIDs);
    ReadVector(in, mesh.weights);

    // 4. Indices
    ReadVector(in, mesh.indices);
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
            importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);

            const aiScene* scene = importer.ReadFile(sourcePath,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenNormals |
                aiProcess_LimitBoneWeights |
                aiProcess_GlobalScale |
                aiProcess_ValidateDataStructure |
                aiProcess_PopulateArmatureData);

            if (!scene || !scene->mRootNode) {
                std::cerr << "[Importer] Assimp Error: " << importer.GetErrorString() << std::endl;
                return { AssetType::Null, "" };
            }

            if (scene->HasAnimations()) {
                std::cout << "[Importer] Found " << scene->mNumAnimations << " animations." << std::endl;

                for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                    aiAnimation* anim = scene->mAnimations[i];

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

MeshData AssetSerializer::ProcessMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData* data = nullptr) {
    MeshData m;

    // 1. Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::ivec4 boneIDs;
        glm::vec4 weights;

        // Set default values
        for (int i = 0; i < 4; i++)
        {
            boneIDs[i] = -1;
            weights[i] = 0.0f;
        }

        m.boneIDs.push_back(boneIDs);
        m.weights.push_back(weights);

        glm::vec3 vector;
        // Set positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        m.vertices.push_back(vector);

        if (mesh->HasNormals())
        {
            // Set normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            m.Normals.push_back(vector);
        }

        if (mesh->mTextureCoords[0])
        {
            // Set texture coords
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            m.TexCoords.push_back(vec);
            if (mesh->HasTangentsAndBitangents()) {
                // Set tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                m.Tangents.push_back(vector);
                // Set bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                m.Bitangents.push_back(vector);
            }
        }
    }
    // Set indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            m.indices.push_back(face.mIndices[j]);
    }

    extractBoneWeightForVertices(
        reinterpret_cast<std::vector<glm::ivec4>&>(m.boneIDs),
        reinterpret_cast<std::vector<glm::vec4>&>(m.weights),
        mesh,
        scene,
        data // Not needed for static mesh
	);

    return m;
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

    // 1. Channels
    for (unsigned int i = 0; i < anim->mNumChannels; i++) {
        aiNodeAnim* channel = anim->mChannels[i];
        SerializedBoneAnim boneAnim;
        boneAnim.name = channel->mNodeName.C_Str();

        // Process Position Keys
        for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
            float time = (float)channel->mPositionKeys[k].mTime;
            glm::vec3 val = Vec3FromAssimp(channel->mPositionKeys[k].mValue);
            boneAnim.positions.push_back({ time, val });
        }

        // Process Rotation Keys - THIS IS THE FIX
        for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
            const aiQuaternion& aiQuat = channel->mRotationKeys[k].mValue;
            float time = (float)channel->mRotationKeys[k].mTime;

            // Validate quaternion before using it
            if (std::isfinite(aiQuat.w) && std::isfinite(aiQuat.x) &&
                std::isfinite(aiQuat.y) && std::isfinite(aiQuat.z)) {

                // Use the helper function for consistency
                glm::quat rotation = QuatFromAssimp(aiQuat);

                // Normalize to ensure it's a valid rotation
                rotation = glm::normalize(rotation);

                boneAnim.rotations.push_back({ time, rotation });
            }
            else {
                std::cerr << "[Animation] Warning: Invalid quaternion in channel "
                    << boneAnim.name << " at key " << k
                    << " (w=" << aiQuat.w << ", x=" << aiQuat.x
                    << ", y=" << aiQuat.y << ", z=" << aiQuat.z << ")\n";

                // Use identity quaternion as fallback
                boneAnim.rotations.push_back({ time, glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
            }
        }

        // Process Scale Keys
        for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
            float time = (float)channel->mScalingKeys[k].mTime;
            glm::vec3 val = Vec3FromAssimp(channel->mScalingKeys[k].mValue);
            boneAnim.scales.push_back({ time, val });
        }

        outAnim.channels.push_back(boneAnim);
    }

    // 2. Hierarchy
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
            min = glm::min(min, v);
            max = glm::max(max, v);
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

    // 1. Header
    AssetHeader header;
    header.magic = ASSET_MAGIC;
    header.type = AssetType::StaticMesh;
    header.version = 1;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    // 2. Global AABB
    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));

    // 3. Meshes
    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (const auto& mesh : data.meshes) {
        WriteMeshData(out, mesh);
    }

    return true;
}

std::shared_ptr<StaticMeshData> AssetSerializer::LoadStaticMesh(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC || header.type != AssetType::StaticMesh) {
        std::cerr << "[Loader] Error: Invalid Static Mesh Header for " << path << std::endl;
        return nullptr;
    }

    auto result = std::make_shared<StaticMeshData>();
    result->path = path;

    // 2. Global AABB
    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));

    // 3. Meshes
    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    result->meshes.resize(meshCount);
    for (uint32_t i = 0; i < meshCount; ++i) {
        ReadMeshData(in, result->meshes[i]);
    }

    return result;
}


std::shared_ptr<SkeletalMeshData> AssetSerializer::ImportSkeletalMeshAssimp(const std::string& path) {
    Assimp::Importer importer;

    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
        aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS);

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_LimitBoneWeights |
        aiProcess_GlobalScale);

    if (!scene || !scene->mRootNode) {
        std::cerr << "[Importer] Failed to load: " << path << "\n";
        return nullptr;
    }

    auto data = std::make_shared<SkeletalMeshData>();
    data->path = path;

    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);


    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMeshPtr = scene->mMeshes[i];

        MeshData meshData = ProcessMesh(aiMeshPtr, scene, data.get());
        for (const auto& v : meshData.vertices) {
            min = glm::min(min, v);
            max = glm::max(max, v);
        }
        data->meshes.push_back(meshData);
    }

    data->aabbMin = min;
    data->aabbMax = max;

    return data;
}

bool AssetSerializer::SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;


    AssetHeader header;
    header.magic = ASSET_MAGIC;
    header.type = AssetType::SkeletalMesh;
    header.version = 1;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));


    out.write(reinterpret_cast<const char*>(&data.aabbMin), sizeof(glm::vec3));
    out.write(reinterpret_cast<const char*>(&data.aabbMax), sizeof(glm::vec3));


    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    out.write(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    for (const auto& mesh : data.meshes) {
        WriteMeshData(out, mesh);
    }


    uint32_t boneCount = static_cast<uint32_t>(data.boneInfoMap.size());
    out.write(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

    std::cout << "\n=== SAVING SKELETAL MESH ===\n";
    std::cout << "Saving " << boneCount << " bones to: " << path << "\n";

    for (const auto& bone : data.boneInfoMap) {

        uint32_t nameLen = static_cast<uint32_t>(bone.name.size());
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
        if (nameLen > 0) out.write(bone.name.c_str(), nameLen);

        out.write(reinterpret_cast<const char*>(&bone.offset), sizeof(glm::mat4));
    }

    std::cout << "=== SAVE COMPLETE ===\n\n";
    return true;
}

std::shared_ptr<SkeletalMeshData> AssetSerializer::LoadSkeletalMesh(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "[Loader] Failed to open: " << path << "\n";
        return nullptr;
    }

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC || header.type != AssetType::SkeletalMesh) {
        std::cerr << "[Loader] Invalid Skeletal Mesh Header for " << path << "\n";
        return nullptr;
    }

    auto result = std::make_shared<SkeletalMeshData>();
    result->path = path;

    in.read(reinterpret_cast<char*>(&result->aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&result->aabbMax), sizeof(glm::vec3));


    uint32_t meshCount = 0;
    in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    result->meshes.resize(meshCount);
    for (uint32_t i = 0; i < meshCount; ++i) {
        ReadMeshData(in, result->meshes[i]);
    }

    uint32_t boneMapSize = 0;
    in.read(reinterpret_cast<char*>(&boneMapSize), sizeof(uint32_t));

    std::cout << "\n=== LOADING SKELETAL MESH ===\n";
    std::cout << "File: " << path << "\n";
    std::cout << "Loading " << boneMapSize << " bones\n";

    result->boneInfoMap.reserve(boneMapSize);

    for (uint32_t i = 0; i < boneMapSize; ++i) {
        BoneProps info;

        uint32_t nameLen = 0;
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
        if (nameLen > 0) {
            info.name.resize(nameLen);
            in.read(&info.name[0], nameLen);
        }

        in.read(reinterpret_cast<char*>(&info.offset), sizeof(glm::mat4));

        result->boneInfoMap.push_back(info);
    }

    result->boneCount = boneMapSize;
    std::cout << "=== LOAD COMPLETE ===\n\n";

    return result;
}

void WriteSerializedNode(std::ofstream& out, const SerializedNode& node) {
    uint32_t nameLen = (uint32_t)node.name.size();
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) out.write(node.name.c_str(), nameLen);

    out.write(reinterpret_cast<const char*>(&node.transformation), sizeof(glm::mat4));

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
    header.type = AssetType::Animation;
    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    out.write(reinterpret_cast<const char*>(&data.duration), sizeof(float));
    out.write(reinterpret_cast<const char*>(&data.ticksPerSecond), sizeof(float));

    uint32_t nameLen = (uint32_t)data.name.size();
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) out.write(data.name.c_str(), nameLen);

    uint32_t numChannels = (uint32_t)data.channels.size();
    out.write(reinterpret_cast<const char*>(&numChannels), sizeof(uint32_t));

    for (const auto& channel : data.channels) {
        uint32_t bLen = (uint32_t)channel.name.size();
        out.write(reinterpret_cast<const char*>(&bLen), sizeof(uint32_t));
        if (bLen > 0) out.write(channel.name.c_str(), bLen);

        uint32_t nPos = (uint32_t)channel.positions.size();
        out.write(reinterpret_cast<const char*>(&nPos), sizeof(uint32_t));
        for (const auto& kv : channel.positions) {
            out.write(reinterpret_cast<const char*>(&kv.first), sizeof(float));
            out.write(reinterpret_cast<const char*>(&kv.second), sizeof(glm::vec3));
        }
        uint32_t nRot = (uint32_t)channel.rotations.size();
        out.write(reinterpret_cast<const char*>(&nRot), sizeof(uint32_t));
        for (const auto& kv : channel.rotations) {
            out.write(reinterpret_cast<const char*>(&kv.first), sizeof(float));
            out.write(reinterpret_cast<const char*>(&kv.second), sizeof(glm::quat));
        }
        uint32_t nScl = (uint32_t)channel.scales.size();
        out.write(reinterpret_cast<const char*>(&nScl), sizeof(uint32_t));
        for (const auto& kv : channel.scales) {
            out.write(reinterpret_cast<const char*>(&kv.first), sizeof(float));
            out.write(reinterpret_cast<const char*>(&kv.second), sizeof(glm::vec3));
        }
    }

    WriteSerializedNode(out, data.rootNode);
    return true;
}


bool AssetSerializer::ImportTexture(const std::string& source, const std::string& dest) {
    int w, h, c;
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

    AssetHeader header;
    header.magic = ASSET_MAGIC;
    header.version = ASSET_VERSION;
    header.type = AssetType::Texture;

    TextureHeader texHeader;
    texHeader.width = static_cast<uint32_t>(w);
    texHeader.height = static_cast<uint32_t>(h);
    texHeader.channels = 4;
    texHeader.dataSize = texHeader.width * texHeader.height * texHeader.channels;

    header.dataSize = sizeof(TextureHeader) + texHeader.dataSize;

    out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    out.write(reinterpret_cast<char*>(&texHeader), sizeof(TextureHeader));
    out.write(reinterpret_cast<char*>(data), texHeader.dataSize);

    out.close();
    stbi_image_free(data);
    return true;
}