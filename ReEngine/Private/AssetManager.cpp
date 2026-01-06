#include "AssetManager.h"
#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include "Animation/Animation.h"
#include "Animation/AssimpNodeData.h"

namespace fs = std::filesystem;

// Extension for your custom cooked assets
const std::string COOKED_EXT = ".rasset";

AssetManager::AssetManager(ThreadPool* threadPool) : pool(threadPool) {}

AssetManager::~AssetManager() { shutdown(); }

void AssetManager::shutdown() {
    std::lock_guard<std::mutex> lock(assetMutex);
    meshCache.clear();
    textureCache.clear();
}


// --- NEW HELPER: Recursive Node Read ---
void ReadSerializedNode(std::ifstream& in, AssimpNodeData& node) {
    // Name
    uint32_t nameLen;
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) {
        node.name.resize(nameLen);
        in.read(&node.name[0], nameLen);
    }

    // Transform
    in.read(reinterpret_cast<char*>(&node.transformation), sizeof(glm::mat4));

    // Children
    uint32_t childCount;
    in.read(reinterpret_cast<char*>(&childCount), sizeof(uint32_t));
    node.childrenCount = (int)childCount;

    for (uint32_t i = 0; i < childCount; ++i) {
        AssimpNodeData child;
        ReadSerializedNode(in, child);
        node.children.push_back(child);
    }
}
// -----------------------------------------------------------------------
//  UPLOAD SYSTEM (Render Thread)
// -----------------------------------------------------------------------

void AssetManager::EnqueueUpload(std::function<void()> func) {
    std::lock_guard<std::mutex> lock(uploadMutex);
    uploadQueue.push(std::move(func));
}

void AssetManager::DispatchUploads() {
    // Standard double-buffer queue swap pattern
    std::queue<std::function<void()>> localQueue;
    {
        std::lock_guard<std::mutex> lock(uploadMutex);
        if (uploadQueue.empty()) return;
        std::swap(localQueue, uploadQueue);
    }
    while (!localQueue.empty()) {
        localQueue.front()();
        localQueue.pop();
    }
}

// -----------------------------------------------------------------------
//  STATIC MESH (Async Load)
// -----------------------------------------------------------------------

std::shared_ptr<MeshResource> AssetManager::CreateManualMesh(const std::string& name, std::shared_ptr<StaticMeshData> data) {
    std::lock_guard<std::mutex> lock(assetMutex);

        auto resource = std::make_shared<MeshResource>();
        resource->cpuMesh = data;
        resource->uploaded = false;
        meshCache[name] = resource;

        // Directly Enqueue the Upload (This will run on the Render Thread)
        this->EnqueueUpload([resource]() {
            if (!resource->cpuMesh) return;

    size_t numSubMeshes = resource->cpuMesh->meshes.size();
        resource->VAOs.resize(numSubMeshes);
        resource->VBOs.resize(numSubMeshes);
        resource->EBOs.resize(numSubMeshes);
        resource->indexCounts.resize(numSubMeshes);

        for (size_t i = 0; i < numSubMeshes; ++i) {
            auto& mesh = resource->cpuMesh->meshes[i];

                glGenVertexArrays(1, &resource->VAOs[i]);
                glGenBuffers(1, &resource->VBOs[i]);
                glGenBuffers(1, &resource->EBOs[i]);

                glBindVertexArray(resource->VAOs[i]);

                glBindBuffer(GL_ARRAY_BUFFER, resource->VBOs[i]);
                glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource->EBOs[i]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

                // Attribute pointers (Position, Normal, UV, etc.)
                glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position)); 
                glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
                glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

                resource->indexCounts[i] = (uint32_t)mesh.indices.size();
        }

    glBindVertexArray(0);
        resource->uploaded = true;
            });

    return resource;
}

std::shared_ptr<MeshResource> AssetManager::GetMesh(const std::string& path) {
    std::lock_guard<std::mutex> lock(assetMutex);
    // Return existing handle if already requested
    if (meshCache.find(path) != meshCache.end()) return meshCache[path];

    // Create a new handle that is initially "Not Uploaded"
    auto resource = std::make_shared<MeshResource>();
    resource->uploaded = false;
    meshCache[path] = resource;

    // Submit Job to IO Thread (JobType::Background)
    pool->submit(JobType::Background, [this, path, resource]() {

        std::string cookedPath = path;

        // This heavy IO happens on background thread
        auto cpuMesh = LoadBinaryStaticMesh(cookedPath);

        if (!cpuMesh) {
            std::cerr << "[AssetManager] Failed to load cooked mesh: " << cookedPath << "\n";
            return;
        }
        resource->cpuMesh = cpuMesh;

        // Once IO is done, queue the GPU Upload for the Main Thread
        this->EnqueueUpload([resource]() {
            if (!resource->cpuMesh) return;

            // Clear and resize to match the number of sub-meshes
            size_t numSubMeshes = resource->cpuMesh->meshes.size();
            resource->VAOs.resize(numSubMeshes);
            resource->VBOs.resize(numSubMeshes);
            resource->EBOs.resize(numSubMeshes);
            resource->indexCounts.resize(numSubMeshes);

            for (size_t i = 0; i < numSubMeshes; ++i) {
                auto& mesh = resource->cpuMesh->meshes[i];

                glGenVertexArrays(1, &resource->VAOs[i]);
                glGenBuffers(1, &resource->VBOs[i]);
                glGenBuffers(1, &resource->EBOs[i]);

                glBindVertexArray(resource->VAOs[i]);

                glBindBuffer(GL_ARRAY_BUFFER, resource->VBOs[i]);
                glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource->EBOs[i]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

                // Attributes
                glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
                glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
                glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
                glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
                glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

                resource->indexCounts[i] = (uint32_t)mesh.indices.size();
            }

            glBindVertexArray(0);
            resource->uploaded = true;
            });
        });

    return resource;
}

// -----------------------------------------------------------------------
//  TEXTURE (Async Load)
// -----------------------------------------------------------------------

std::shared_ptr<TextureResource> AssetManager::GetTexture(const std::string& rawPath) {
    std::string path = rawPath;
    std::replace(path.begin(), path.end(), '\\', '/');

    std::lock_guard<std::mutex> lock(assetMutex);
    if (textureCache.find(path) != textureCache.end()) return textureCache[path];

    auto resource = std::make_shared<TextureResource>();
    resource->id = lastTextureResourceId++;
    resource->uploaded = false;
    textureCache[path] = resource;

    // Submit to IO Thread
    pool->submit(JobType::Background, [this, path, resource]() {
        std::string cookedPath = path;
        std::shared_ptr<TextureLoadResult> texData = LoadBinaryTexture(cookedPath);

        if (!texData) {
            std::cerr << "[AssetManager] Failed to load cooked texture: " << cookedPath << "\n";
            return;
        }

        // Enqueue Upload to Main Thread
        this->EnqueueUpload([resource, texData = std::move(texData)]() {
            resource->width = texData->w; resource->height = texData->h;
            glGenTextures(1, &resource->id);
            glBindTexture(GL_TEXTURE_2D, resource->id);
            // Note: Uploading raw pixels can be slow. In a pro engine, 
            // you might use PBOs here to keep this async on the GPU too.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texData->w, texData->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData->pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            resource->uploaded = true;
            });
        });
    return resource;
}

// -----------------------------------------------------------------------
//  BINARY LOADERS (Same as previous turn, just ensuring implementation exists)
// -----------------------------------------------------------------------

std::shared_ptr<StaticMeshData> AssetManager::LoadBinaryStaticMesh(const std::string& path) {
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

        for (const auto& v : result->meshes[i].vertices) {
            result->meshes[i].aabbMin = glm::min(result->meshes[i].aabbMin, v.Position);
            result->meshes[i].aabbMax = glm::max(result->meshes[i].aabbMax, v.Position);
        }
    }
    return result;
}

std::unique_ptr<AssetManager::TextureLoadResult> AssetManager::LoadBinaryTexture(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    if (header.magic != ASSET_MAGIC || header.type != AssetType::Texture) return nullptr;

    TextureHeader texHeader;
    in.read(reinterpret_cast<char*>(&texHeader), sizeof(TextureHeader));

    auto result = std::make_unique<TextureLoadResult>();
    result->w = texHeader.width;
    result->h = texHeader.height;
    result->c = texHeader.channels;
    result->pixels.resize(texHeader.dataSize);

    in.read(reinterpret_cast<char*>(result->pixels.data()), texHeader.dataSize);
    return result;
}

std::shared_ptr<Animation> AssetManager::LoadBinaryAnimation(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "[AssetManager] Failed to open animation file: " << path << "\n";
        return nullptr;
    }

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

    if (header.magic != ASSET_MAGIC) {
        std::cerr << "[AssetManager] Invalid Magic Header in: " << path << "\n";
        return nullptr;
    }

    // 1. Read Metadata
    float duration, tps;
    in.read(reinterpret_cast<char*>(&duration), sizeof(float));
    in.read(reinterpret_cast<char*>(&tps), sizeof(float));

    std::cout << "[AssetManager] Loading Animation - Duration: " << duration << ", TPS: " << tps << "\n";

    // 2. Read Animation Name
    uint32_t nameLen;
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
    std::string animName;
    if (nameLen > 0 && nameLen < 500) {
        animName.resize(nameLen);
        in.read(&animName[0], nameLen);
        std::cout << "[AssetManager] Animation Name: " << animName << "\n";
    }

    auto animation = std::make_shared<Animation>();
    animation->SetDuration(duration);
    animation->SetTicksPerSecond((int)tps);

    // 3. Read Channels (Bone Animations)
    uint32_t numChannels;
    in.read(reinterpret_cast<char*>(&numChannels), sizeof(uint32_t));
    std::cout << "[AssetManager] Loading " << numChannels << " animation channels\n";

    for (uint32_t i = 0; i < numChannels; ++i) {
        // Read Bone Name
        uint32_t boneNameLen;
        in.read(reinterpret_cast<char*>(&boneNameLen), sizeof(uint32_t));

        std::string boneName;
        if (boneNameLen > 0 && boneNameLen < 500) {
            boneName.resize(boneNameLen);
            in.read(&boneName[0], boneNameLen);
        }
        else {
            std::cerr << "[AssetManager] Invalid bone name length: " << boneNameLen << "\n";
            return nullptr;
        }

        std::vector<KeyPosition> positions;
        std::vector<KeyRotation> rotations;
        std::vector<KeyScale> scales;

        // Read Position Keys
        uint32_t numPos;
        in.read(reinterpret_cast<char*>(&numPos), sizeof(uint32_t));
        for (uint32_t k = 0; k < numPos; k++) {
            float timestamp;
            glm::vec3 value;
            in.read(reinterpret_cast<char*>(&timestamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&value), sizeof(glm::vec3));
            positions.push_back({ value, timestamp }); // Note: struct is {vec3, float}
        }

        // Read Rotation Keys
        uint32_t numRot;
        in.read(reinterpret_cast<char*>(&numRot), sizeof(uint32_t));
        for (uint32_t k = 0; k < numRot; k++) {
            float timestamp;
            glm::quat value;
            in.read(reinterpret_cast<char*>(&timestamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&value), sizeof(glm::quat));
            rotations.push_back({ value, timestamp }); // struct is {quat, float}
        }

        // Read Scale Keys
        uint32_t numScl;
        in.read(reinterpret_cast<char*>(&numScl), sizeof(uint32_t));
        for (uint32_t k = 0; k < numScl; k++) {
            float timestamp;
            glm::vec3 value;
            in.read(reinterpret_cast<char*>(&timestamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&value), sizeof(glm::vec3));
            scales.push_back({ value, timestamp }); // struct is {vec3, float}
        }

        std::cout << "  Channel " << i << " (" << boneName << "): "
            << numPos << " pos, " << numRot << " rot, " << numScl << " scale keys\n";

        // **CRITICAL: Use the bone ID as index, not the channel index**
        // Pass -1 as ID for now; the Animator will look up by name
        animation->AddBone(Bone(boneName, -1, positions, rotations, scales));
    }

    // 4. Read Hierarchy
    AssimpNodeData rootNode;
    ReadSerializedNode(in, rootNode);
    animation->SetRootNode(rootNode);

    std::cout << "[AssetManager] Animation loaded successfully. Root node: " << rootNode.name << "\n";

    return animation;
}

std::shared_ptr<MeshResource> AssetManager::GetSkeletalMesh(const std::string& path) {
    std::lock_guard<std::mutex> lock(assetMutex);
    if (meshCache.find(path) != meshCache.end()) return meshCache[path];

    auto resource = std::make_shared<MeshResource>();
    resource->uploaded = false;
    meshCache[path] = resource;

    pool->submit(JobType::Background, [this, path, resource]() {
        std::string cookedPath = path;
        auto cpuMesh = LoadBinarySkeletalMesh(cookedPath);
        if (!cpuMesh) return;

        resource->cpuMesh = cpuMesh;

        this->EnqueueUpload([resource, cpuMesh]() {
            size_t numMeshes = cpuMesh->meshes.size();

            // Initialize vectors
            resource->VAOs.resize(numMeshes);
            resource->VBOs.resize(numMeshes);
            resource->EBOs.resize(numMeshes);
            resource->BVAOs.resize(numMeshes);
            resource->indexCounts.resize(numMeshes);

            for (size_t i = 0; i < numMeshes; ++i) {
                auto& mesh = cpuMesh->meshes[i];

                glGenVertexArrays(1, &resource->VAOs[i]);
                glGenBuffers(1, &resource->VBOs[i]);
                glGenBuffers(1, &resource->EBOs[i]);

                glBindVertexArray(resource->VAOs[i]);

                // 1. Standard Vertex Data (Pos, Normal, UV, etc.)
                glBindBuffer(GL_ARRAY_BUFFER, resource->VBOs[i]);
                glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource->EBOs[i]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

                // Attributes 0-4
                glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
                glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
                glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
                glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
                glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

                // 2. Skeletal Bone Data (IDs and Weights)
                if (i < cpuMesh->bonesPerMesh.size()) {
                    glGenBuffers(1, &resource->BVAOs[i]);
                    glBindBuffer(GL_ARRAY_BUFFER, resource->BVAOs[i]);

                    auto& boneData = cpuMesh->bonesPerMesh[i];
                    glBufferData(GL_ARRAY_BUFFER, boneData.size() * sizeof(VertexBoneData), boneData.data(), GL_STATIC_DRAW);

                    // Attribute 5: Bone IDs (Note: glVertexAttribIPointer for Integers!)
                    glEnableVertexAttribArray(5);
                    glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexBoneData), (void*)offsetof(VertexBoneData, BoneIDs));

                    // Attribute 6: Weights
                    glEnableVertexAttribArray(6);
                    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)offsetof(VertexBoneData, Weights));
                }

                resource->indexCounts[i] = (uint32_t)mesh.indices.size();
            }

            glBindVertexArray(0);
            resource->uploaded = true;
            });
        });

    return resource;
}

std::shared_ptr<SkeletalMeshData> AssetManager::LoadBinarySkeletalMesh(const std::string& path) {
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
        std::string boneName; boneName.resize(nameLen);
        in.read(&boneName[0], nameLen);
        BoneInfo info;
        in.read(reinterpret_cast<char*>(&info), sizeof(BoneInfo));
        result->boneInfoMap[boneName] = info;
    }
    result->boneCount = boneMapSize;
    return result;
}

std::shared_ptr<AnimationGraphResource> AssetManager::GetAnimationGraph(const std::string& path)
{
    std::lock_guard<std::mutex> lock(assetMutex);

    // 1. Check Cache
    if (graphCache.find(path) != graphCache.end()) {
        return graphCache[path];
    }

    // 2. Load from Disk
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[AssetManager] Failed to load Animation Graph: " << path << "\n";
        return nullptr;
    }

    json j;
    try {
        file >> j;
    }
    catch (...) {
        std::cerr << "[AssetManager] JSON Parsing Error in: " << path << "\n";
        return nullptr;
    }

    auto newGraph = std::make_shared<AnimationGraphResource>();

    // Basic Info
    if (j.contains("entryNodeId")) newGraph->EntryNodeID = j["entryNodeId"];

    // Parameters / Blackboard Defaults
    if (j.contains("parameters")) {
        if (j["parameters"].contains("floats")) {
            for (auto& [key, val] : j["parameters"]["floats"].items())
                newGraph->DefaultBlackboard[key] = AnimVar(val.get<float>());
        }
        if (j["parameters"].contains("bools")) {
            for (auto& [key, val] : j["parameters"]["bools"].items())
                newGraph->DefaultBlackboard[key] = AnimVar(val.get<bool>());
        }
    }

    // Nodes
    if (j.contains("nodes")) {
        for (auto& jNode : j["nodes"]) {
            GraphNode node;
            node.ID = jNode["id"];
            if (jNode.contains("name")) node.Name = jNode["name"];
            else if (jNode.contains("animName")) node.Name = jNode["animName"];
            if (jNode.contains("animPath")) node.AnimationPath = jNode["animPath"];
            // We don't necessarily need EditorPosition for runtime, but can load it if needed
            newGraph->Nodes.push_back(node);
        }
    }

    // Transitions
    if (j.contains("transitions")) {
        for (auto& jTrans : j["transitions"]) {
            GraphTransition trans;
            trans.ID = jTrans["id"];
            trans.FromNodeID = jTrans["from"];
            trans.ToNodeID = jTrans["to"];
            trans.ConditionParam = jTrans["condition"];
            trans.Threshold = jTrans["threshold"];
            trans.Operation = (ConditionOp)jTrans["op"];
            newGraph->Transitions.push_back(trans);
        }
    }

    // 3. Cache and Return
    graphCache[path] = newGraph;
    return newGraph;
}

std::shared_ptr<Animation> AssetManager::GetAnimation(const std::string& rawPath) {
    std::string path = rawPath;
    std::replace(path.begin(), path.end(), '\\', '/'); // Normalize

    std::lock_guard<std::mutex> lock(assetMutex);

    // 1. Check Cache
    if (animationCache.find(path) != animationCache.end()) {
        return animationCache[path];
    }

    // 2. Load from Disk
    std::shared_ptr<Animation> anim = LoadBinaryAnimation(path);

    if (anim) {
        animationCache[path] = anim;
    }
    else {
        std::cerr << "[AssetManager] Failed to load animation: " << path << "\n";
    }

    return anim;
}

// Utilities
void AssetManager::addMaterial(int id, CompiledMaterial material) { materials[id] = material; }
CompiledMaterial* AssetManager::GetMaterial(int id) { return materials.count(id) ? &materials[id] : nullptr; }
void AssetManager::unloadTexture(const std::string& path) { std::lock_guard<std::mutex> l(assetMutex); textureCache.erase(path); }
void AssetManager::unloadMesh(const std::string& path) { std::lock_guard<std::mutex> l(assetMutex); meshCache.erase(path); }

std::vector<std::string> AssetManager::GetCachedPaths() {
    std::lock_guard<std::mutex> l(assetMutex);
    std::vector<std::string> p; for (auto& kv : meshCache) p.push_back(kv.first); return p;
}
std::vector<std::string> AssetManager::GetCachedTexturesPaths() {
    std::lock_guard<std::mutex> l(assetMutex);
    std::vector<std::string> p; for (auto& kv : textureCache) p.push_back(kv.first); return p;
}