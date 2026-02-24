#include "AssetManager.h"
#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Animation/Animation.h"
#include "Animation/AssimpNodeData.h"

namespace fs = std::filesystem;

AssetManager::AssetManager(ThreadPool* threadPool) : pool(threadPool) {}

AssetManager::~AssetManager() { shutdown(); }

void AssetManager::LoadMaterial(int id, const std::string& path) {
    Material tempMat;
    if (!tempMat.LoadFromFile(path)) {
        std::cerr << "[AssetManager] Failed to load material file: " << path << "\n";
        return;
    }
    CompiledMaterial compiled = tempMat.Compile(this);
    compiled.path = path;

    this->EnqueueUpload([this, id, compiled = std::move(compiled)]() mutable {
        compiled.BuildGLShader();
        {
            std::lock_guard<std::mutex> lock(assetMutex);
            addMaterial(compiled.id, compiled);
        }
        });
}

void AssetManager::shutdown() {
    std::lock_guard<std::mutex> lock(assetMutex);
    meshCache.clear();
    textureCache.clear();
}




void AssetManager::ReadSerializedNode(std::ifstream& in, AssimpNodeData& node) {
    uint32_t nameLen;
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) {
        node.name.resize(nameLen);
        in.read(&node.name[0], nameLen);
    }

    in.read(reinterpret_cast<char*>(&node.transformation), sizeof(glm::mat4));

    uint32_t childCount;
    in.read(reinterpret_cast<char*>(&childCount), sizeof(uint32_t));
    node.childrenCount = (int)childCount;

    for (uint32_t i = 0; i < childCount; ++i) {
        AssimpNodeData child;
        ReadSerializedNode(in, child);
        node.children.push_back(child);
    }
}




void AssetManager::ReadMeshData(std::ifstream& in, MeshData& mesh) {
    in.read(reinterpret_cast<char*>(&mesh.materialIndex), sizeof(int));
    in.read(reinterpret_cast<char*>(&mesh.aabbMin), sizeof(glm::vec3));
    in.read(reinterpret_cast<char*>(&mesh.aabbMax), sizeof(glm::vec3));

    ReadVector(in, mesh.vertices);
    ReadVector(in, mesh.Normals);
    ReadVector(in, mesh.TexCoords);
    ReadVector(in, mesh.Tangents);
    ReadVector(in, mesh.Bitangents);
    ReadVector(in, mesh.boneIDs);
    ReadVector(in, mesh.weights);
    ReadVector(in, mesh.indices);
}





void AssetManager::EnqueueUpload(std::function<void()> func) {
    std::lock_guard<std::mutex> lock(uploadMutex);
    uploadQueue.push(std::move(func));
}

void AssetManager::DispatchUploads() {
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





std::shared_ptr<MeshResource> AssetManager::CreateManualMesh(const std::string& name, std::shared_ptr<StaticMeshData> data) {
    std::lock_guard<std::mutex> lock(assetMutex);

    auto resource = std::make_shared<MeshResource>();
    resource->cpuMesh = data;
    resource->uploaded = false;
    meshCache[name] = resource;

    this->EnqueueUpload([resource]() {
        if (!resource->cpuMesh) return;
        size_t numSubMeshes = resource->cpuMesh->meshes.size();
        resource->VAOs.resize(numSubMeshes);
        resource->VBOs.resize(numSubMeshes);
        resource->EBOs.resize(numSubMeshes);
        resource->indexCounts.resize(numSubMeshes);

        for (size_t i = 0; i < numSubMeshes; ++i) {
            auto& mesh = resource->cpuMesh->meshes[i];
            GPUBuffers buffers = generateBuffer(mesh);
            resource->VAOs[i] = buffers.VAO;
            resource->VBOs[i] = buffers.VBOs;
            resource->EBOs[i] = buffers.EBO;
            resource->indexCounts[i] = (uint32_t)mesh.indices.size();
        }
        glBindVertexArray(0);
        resource->uploaded = true;
        });

    return resource;
}

std::shared_ptr<MeshResource> AssetManager::GetMesh(const std::string& path) {
    std::lock_guard<std::mutex> lock(assetMutex);
    if (meshCache.find(path) != meshCache.end()) return meshCache[path];

    auto resource = std::make_shared<MeshResource>();
    resource->uploaded = false;
    meshCache[path] = resource;

    pool->submit(JobType::Background, [this, path, resource]() {
        auto cpuMesh = LoadBinaryStaticMesh(path);
        if (!cpuMesh) {
            std::cerr << "[AssetManager] Failed to load cooked mesh: " << path << "\n";
            return;
        }
        resource->cpuMesh = cpuMesh;

        this->EnqueueUpload([resource]() {
            if (!resource->cpuMesh) return;
            size_t numSubMeshes = resource->cpuMesh->meshes.size();
            resource->VAOs.resize(numSubMeshes);
            resource->VBOs.resize(numSubMeshes);
            resource->EBOs.resize(numSubMeshes);
            resource->indexCounts.resize(numSubMeshes);

            for (size_t i = 0; i < numSubMeshes; ++i) {
                auto& mesh = resource->cpuMesh->meshes[i];
                GPUBuffers buffers = generateBuffer(mesh);
                resource->VAOs[i] = buffers.VAO;
                resource->VBOs[i] = buffers.VBOs;
                resource->EBOs[i] = buffers.EBO;
                resource->indexCounts[i] = (uint32_t)mesh.indices.size();
            }
            glBindVertexArray(0);
            resource->uploaded = true;
            });
        });

    return resource;
}

std::shared_ptr<MeshResource> AssetManager::GetSkeletalMesh(const std::string& path) {
    std::lock_guard<std::mutex> lock(assetMutex);
    if (meshCache.find(path) != meshCache.end()) return meshCache[path];

    auto resource = std::make_shared<MeshResource>();
    resource->uploaded = false;
    meshCache[path] = resource;

    pool->submit(JobType::Background, [this, path, resource]() {
        auto cpuMesh = LoadBinarySkeletalMesh(path);
        if (!cpuMesh) return;
        resource->cpuMesh = cpuMesh;

        this->EnqueueUpload([resource, cpuMesh]() {
            size_t numSubMeshes = resource->cpuMesh->meshes.size();
            resource->VAOs.resize(numSubMeshes);
            resource->VBOs.resize(numSubMeshes);
            resource->EBOs.resize(numSubMeshes);
            resource->indexCounts.resize(numSubMeshes);

            for (size_t i = 0; i < numSubMeshes; ++i) {
                auto& mesh = resource->cpuMesh->meshes[i];
                GPUBuffers buffers = generateBuffer(mesh);
                resource->VAOs[i] = buffers.VAO;
                resource->VBOs[i] = buffers.VBOs;
                resource->EBOs[i] = buffers.EBO;
                resource->indexCounts[i] = (uint32_t)mesh.indices.size();
            }
            glBindVertexArray(0);
            resource->uploaded = true;
            });
        });

    return resource;
}





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
    for (uint32_t i = 0; i < meshCount; ++i) ReadMeshData(in, result->meshes[i]);

    uint32_t physSize = 0;
    if (in.peek() != EOF) {
        in.read(reinterpret_cast<char*>(&physSize), sizeof(uint32_t));
        if (physSize > 0) {
            result->physicsData.resize(physSize);
            in.read(reinterpret_cast<char*>(result->physicsData.data()), physSize);
        }
    }

    return result;
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
    for (uint32_t i = 0; i < meshCount; ++i) ReadMeshData(in, result->meshes[i]);

    uint32_t boneCount = 0;
    in.read(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));
    result->boneCount = boneCount;
    result->boneInfoMap.reserve(boneCount);

    for (uint32_t i = 0; i < boneCount; ++i) {
        BoneProps props;
        uint32_t nameLen = 0;
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
        if (nameLen > 0) {
            props.name.resize(nameLen);
            in.read(&props.name[0], nameLen);
        }
        in.read(reinterpret_cast<char*>(&props.offset), sizeof(glm::mat4));
        result->boneInfoMap.push_back(props);
    }

    return result;
}

std::shared_ptr<Animation> AssetManager::LoadBinaryAnimation(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return nullptr;

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
    if (header.magic != ASSET_MAGIC) return nullptr;

    auto animation = std::make_shared<Animation>();

    float duration, tps;
    in.read(reinterpret_cast<char*>(&duration), sizeof(float));
    in.read(reinterpret_cast<char*>(&tps), sizeof(float));
    animation->SetDuration(duration);
    animation->SetTicksPerSecond(tps);

    
    
    
    uint32_t nameLen;
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
    if (nameLen > 0) {
        std::string animName;
        animName.resize(nameLen);
        in.read(&animName[0], nameLen);
        
    }

    uint32_t numChannels;
    in.read(reinterpret_cast<char*>(&numChannels), sizeof(uint32_t));

    for (uint32_t i = 0; i < numChannels; ++i) {
        uint32_t bNameLen;
        in.read(reinterpret_cast<char*>(&bNameLen), sizeof(uint32_t));
        std::string boneName;
        boneName.resize(bNameLen);
        in.read(&boneName[0], bNameLen);

        std::vector<KeyPosition> positions;
        std::vector<KeyRotation> rotations;
        std::vector<KeyScale>    scales;

        uint32_t numPos;
        in.read(reinterpret_cast<char*>(&numPos), sizeof(uint32_t));
        positions.resize(numPos);
        for (auto& p : positions) {
            in.read(reinterpret_cast<char*>(&p.timeStamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&p.position), sizeof(glm::vec3));
            if (!std::isfinite(p.timeStamp) ||
                !std::isfinite(p.position.x) || !std::isfinite(p.position.y) || !std::isfinite(p.position.z)) {
                p.timeStamp = 0.0f;
                p.position = glm::vec3(0.0f);
            }
        }

        uint32_t numRot;
        in.read(reinterpret_cast<char*>(&numRot), sizeof(uint32_t));
        rotations.resize(numRot);
        for (auto& r : rotations) {
            in.read(reinterpret_cast<char*>(&r.timeStamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&r.orientation), sizeof(glm::quat));
            if (!std::isfinite(r.timeStamp) ||
                !std::isfinite(r.orientation.w) || !std::isfinite(r.orientation.x) ||
                !std::isfinite(r.orientation.y) || !std::isfinite(r.orientation.z)) {
                r.timeStamp = 0.0f;
                r.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            }
            else {
                r.orientation = glm::normalize(r.orientation);
            }
        }

        uint32_t numScl;
        in.read(reinterpret_cast<char*>(&numScl), sizeof(uint32_t));
        scales.resize(numScl);
        for (auto& s : scales) {
            in.read(reinterpret_cast<char*>(&s.timeStamp), sizeof(float));
            in.read(reinterpret_cast<char*>(&s.scale), sizeof(glm::vec3));
            if (!std::isfinite(s.timeStamp) ||
                !std::isfinite(s.scale.x) || !std::isfinite(s.scale.y) || !std::isfinite(s.scale.z)) {
                s.timeStamp = 0.0f;
                s.scale = glm::vec3(1.0f);
            }
        }

        animation->AddBone(Bone(boneName, -1, positions, rotations, scales));
    }

    AssimpNodeData rootNode;
    ReadSerializedNode(in, rootNode);
    animation->SetRootNode(rootNode);

    return animation;
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

std::shared_ptr<TextureResource> AssetManager::GetTexture(const std::string& rawPath) {
    std::string path = rawPath;
    std::replace(path.begin(), path.end(), '\\', '/');

    std::lock_guard<std::mutex> lock(assetMutex);
    if (textureCache.find(path) != textureCache.end()) return textureCache[path];

    auto resource = std::make_shared<TextureResource>();
    resource->id = lastTextureResourceId++;
    resource->uploaded = false;
    textureCache[path] = resource;

    pool->submit(JobType::Background, [this, path, resource]() {
        std::shared_ptr<TextureLoadResult> texData = LoadBinaryTexture(path);
        if (!texData) {
            return;
        }

        this->EnqueueUpload([resource, texData = std::move(texData)]() {
            resource->width = texData->w;
            resource->height = texData->h;
            glGenTextures(1, &resource->id);
            glBindTexture(GL_TEXTURE_2D, resource->id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texData->w, texData->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, texData->pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            resource->uploaded = true;
            });
        });

    return resource;
}





void AssetManager::addMaterial(int id, CompiledMaterial material) { materials[id] = material; }
CompiledMaterial* AssetManager::GetMaterial(int id) { return materials.count(id) ? &materials[id] : nullptr; }
void AssetManager::unloadTexture(const std::string& path) { std::lock_guard<std::mutex> l(assetMutex); textureCache.erase(path); }
void AssetManager::unloadMesh(const std::string& path) { std::lock_guard<std::mutex> l(assetMutex); meshCache.erase(path); }

std::vector<std::string> AssetManager::GetCachedPaths() {
    std::lock_guard<std::mutex> l(assetMutex);
    std::vector<std::string> p;
    for (auto& kv : meshCache) p.push_back(kv.first);
    return p;
}

std::vector<std::string> AssetManager::GetCachedTexturesPaths() {
    std::lock_guard<std::mutex> l(assetMutex);
    std::vector<std::string> p;
    for (auto& kv : textureCache) p.push_back(kv.first);
    return p;
}

std::shared_ptr<AnimationGraphResource> AssetManager::GetAnimationGraph(const std::string& path) {
    std::lock_guard<std::mutex> lock(assetMutex);
    if (graphCache.find(path) != graphCache.end()) return graphCache[path];

    std::ifstream file(path);
    if (!file.is_open()) return nullptr;

    json j;
    try { file >> j; }
    catch (...) { return nullptr; }

    auto newGraph = std::make_shared<AnimationGraphResource>();
    if (j.contains("entryNodeId")) newGraph->EntryNodeID = j["entryNodeId"];

    if (j.contains("parameters")) {
        if (j["parameters"].contains("floats"))
            for (auto& [key, val] : j["parameters"]["floats"].items())
                newGraph->DefaultBlackboard[key] = AnimVar(val.get<float>());
        if (j["parameters"].contains("bools"))
            for (auto& [key, val] : j["parameters"]["bools"].items())
                newGraph->DefaultBlackboard[key] = AnimVar(val.get<bool>());
    }

    if (j.contains("nodes")) {
        for (auto& jNode : j["nodes"]) {
            GraphNode node;
            node.ID = jNode["id"];
            if (jNode.contains("name"))     node.Name = jNode["name"];
            else if (jNode.contains("animName")) node.Name = jNode["animName"];
            if (jNode.contains("animPath")) node.AnimationPath = jNode["animPath"];
            newGraph->Nodes.push_back(node);
        }
    }

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

    graphCache[path] = newGraph;
    return newGraph;
}

std::shared_ptr<Animation> AssetManager::GetAnimation(const std::string& rawPath, SkeletalMeshData* skeletalData) {
    std::string path = rawPath;
    std::replace(path.begin(), path.end(), '\\', '/');

    std::lock_guard<std::mutex> lock(assetMutex);

    
    
    
    
    
    
    if (animationCache.find(path) != animationCache.end()) {
        return animationCache[path];
    }

    auto anim = LoadBinaryAnimation(path);
    if (anim) {
        
        if (skeletalData) {
            anim->LoadIntermediateBones(skeletalData);
        }
        animationCache[path] = anim;
    }
    else {
        std::cerr << "[AssetManager] Failed to load animation: " << path << "\n";
    }

    return anim;
}


