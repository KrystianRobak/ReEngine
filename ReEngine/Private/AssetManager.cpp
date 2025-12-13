#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//static glm::mat4 AssimpToGLM(const aiMatrix4x4& from) {
//    glm::mat4 to;
//    // the a,b,c,d in assimp is row-major, glm is column-major default constructor
//    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
//    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
//    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
//    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
//    return to;
//}

static void SetupMeshOpenGL(MeshResource& r)
{
    // r.cpuMesh must be valid here
    auto& cpu = r.cpuMesh; // StaticMeshData should contain vector<MeshData> meshes or a single MeshData
    // If StaticMeshData has multiple sub-meshes: you can choose to create one combined VAO or store per-submesh resources.
    // For brevity, assume StaticMeshData contains a vector<MeshData> meshes, and we upload each submesh individually.
    // Here we'll upload the first mesh only as an example (adapt as needed).
    if (cpu->meshes.empty()) return;

    // For each MeshData:
    for (auto& mesh : cpu->meshes) {
        // Create VAO/VBO/EBO
        glGenVertexArrays(1, &r.VAO);
        glGenBuffers(1, &r.VBO);
        glGenBuffers(1, &r.EBO);

        glBindVertexArray(r.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, r.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

        // Vertex layout (match your Vertex struct)
        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1); // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2); // TexCoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3); // Tangent
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4); // Bitangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);

        r.indexCount = static_cast<uint32_t>(mesh.indices.size());

        // If you have multiple submeshes you need to store them separately. For simplicity, we're handling one.
        break;
    }
}


static void SetupSkeletalMeshOpenGL(MeshResource& r, SkeletalMeshData* skeletalCpu)
{
    SetupMeshOpenGL(r);

    // We need to flatten the bone data for the specific sub-mesh we are uploading
    // Assuming we are inside the loop iterating over meshes[i]:
    for (int i = 0; i < skeletalCpu->meshes.size(); ++i)
    {
        glGenBuffers(1, &r.BVAO);
        glBindBuffer(GL_ARRAY_BUFFER, r.BVAO);

        auto& boneDataVec = skeletalCpu->bonesPerMesh[i]; // 'i' is the submesh index
        glBufferData(GL_ARRAY_BUFFER, boneDataVec.size() * sizeof(VertexBoneData), boneDataVec.data(), GL_STATIC_DRAW);

        // Attribute 5: Bone IDs (Integers!)
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexBoneData), (void*)offsetof(VertexBoneData, BoneIDs));

        // Attribute 6: Weights (Floats)
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)offsetof(VertexBoneData, Weights));

        glBindVertexArray(0);
    }
}


MeshResourceId AssetManager::RegisterMesh(std::shared_ptr<StaticMeshData> cpuMesh)
{
    std::lock_guard<std::mutex> lock(meshResourcesMutex);
    MeshResourceId id = ++lastMeshResourceId;

    auto res = std::make_shared<MeshResource>();
    res->cpuMesh = std::move(cpuMesh);
    res->needsUpload = true;
    res->uploaded = false;
    res->VAO = res->VBO = res->EBO = 0;
    res->indexCount = 0;

    meshResources.emplace(id, res);

    return id;
}

void AssetManager::UploadPendingResources()
{
    std::vector<std::shared_ptr<TextureData>> texturesToUpload;

    {
        std::lock_guard<std::mutex> lock(gpuTextureMutex);
        texturesToUpload.swap(pendingTextures); // move pending textures out
    }

    for (auto& texData : texturesToUpload)
    {
        if (!texData) continue;

        std::shared_ptr<TextureResource> texRes;

        {
            std::lock_guard<std::mutex> lock(gpuTextureMutex);
            auto it = gpuTextures.find(texData->path);
            if (it != gpuTextures.end())
                texRes = it->second;
        }

        // If first time, create resource
        if (!texRes)
        {
            texRes = std::make_shared<TextureResource>();
            texRes->width = texData->width;
            texRes->height = texData->height;

            glGenTextures(1, &texRes->id);
            glBindTexture(GL_TEXTURE_2D, texRes->id);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                texData->width,
                texData->height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                texData->pixels.data()
            );

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBindTexture(GL_TEXTURE_2D, 0);

            texRes->uploaded = true;

            // store in GPU cache
            {
                std::lock_guard<std::mutex> lock(gpuTextureMutex);
                gpuTextures[texData->path] = texRes;
            }
        }
    }

    std::vector<std::shared_ptr<MeshResource>> toUpload;

    {
        std::lock_guard<std::mutex> lock(meshResourcesMutex);

        for (auto& [id, resPtr] : meshResources) {

            if (resPtr->needsUpload && !resPtr->uploaded) {

                resPtr->needsUpload = false;
                toUpload.push_back(resPtr);
            }
        }
    }

    // Upload outside lock
    for (auto& res : toUpload) {

        if (!res->cpuMesh)
            continue;

        SetupMeshOpenGL(*res);
        res->uploaded = true;
    }
}

MeshResource* AssetManager::GetMeshResource(MeshResourceId id)
{
    std::lock_guard<std::mutex> lock(meshResourcesMutex);
    auto it = meshResources.find(id);

    if (it == meshResources.end())
        return nullptr;

    return it->second.get();
}


std::future<std::shared_ptr<StaticMeshData>> AssetManager::loadFBX(const std::string& path) {
    return pool->submit([this, path]() {
		auto mesh = this->importFBX(path);

        return mesh;
        });
}

std::future<std::shared_ptr<TextureData>> AssetManager::loadTexture(const std::string& rawPath)
{
    std::string path = rawPath;
    std::replace(path.begin(), path.end(), '\\', '/');

    std::lock_guard<std::mutex> lock(textureCacheMutex);

    // If exists in CPU cache
    auto cached = textureCache[path].lock();
    if (cached)
        return std::async(std::launch::deferred, [cached]() { return cached; });

    // Launch async load
    return pool->submit([this, path]() -> std::shared_ptr<TextureData> {

        int w, h, c;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 4);

        if (!data)
            return nullptr;

        auto tex = std::make_shared<TextureData>(w, h, 4, data);
        tex->path = path;

        stbi_image_free(data);

        // put into CPU cache
        {
            std::lock_guard<std::mutex> lock(textureCacheMutex);
            textureCache[path] = tex;
        }

        // mark as pending GPU upload
        {
            std::lock_guard<std::mutex> lock(gpuTextureMutex);
            pendingTextures.push_back(tex);
        }

        return tex;
        });
}

void AssetManager::unloadTexture(const std::string& path)
{
    std::lock_guard<std::mutex> lock(textureCacheMutex);
	textureCache.erase(path);
}

std::vector<std::string> AssetManager::GetCachedTexturesPaths()
{
    // Use the mutex that protects GPU texture access
    std::lock_guard<std::mutex> lock(gpuTextureMutex);
    std::vector<std::string> paths;

    // Iterate over the map holding the shared_ptr<TextureResource>
    for (const auto& pair : gpuTextures)
    {
        // Add the path (key) to the list
        paths.push_back(pair.first);
    }

    return paths;
}

void AssetManager::unloadMesh(const std::string& path)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
	cache.erase(path);
}

void AssetManager::addMaterial(int id, CompiledMaterial material)
{
	materials[id] = material;
}

void AssetManager::AddPendingMesh(Entity entity, std::future<std::shared_ptr<StaticMeshData>> future)
{
    PendingStaticMesh Pending;

	Pending.entity = entity;
	Pending.future = std::move(future);

	pendingMeshes.emplace_back(std::move(Pending));
}

inline std::vector<std::string> AssetManager::GetCachedPaths()
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    std::vector<std::string> paths;
    for (auto& [path, weak] : cache)
        if (!weak.expired())
            paths.push_back(path);
    return paths;
}


std::future<std::shared_ptr<SkeletalMeshData>> AssetManager::loadSkeletalFBX(const std::string& path) {
    return pool->submit([this, path]() {
        return this->importSkeletalMesh(path);
        });
}

std::shared_ptr<SkeletalMeshData> AssetManager::importSkeletalMesh(const std::string& path) {
    // cache check
    {
        std::lock_guard<std::mutex> lock(skeletalCacheMutex);
        if (auto cached = SkeletalMeshCache[path].lock())
            return cached;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp Skeletal Error: " + std::string(importer.GetErrorString()));
    }

    auto skeletalMesh = std::make_shared<SkeletalMeshData>();
    skeletalMesh->path = path;

    // Resize the bonesPerMesh vector to match the number of meshes
    skeletalMesh->bonesPerMesh.resize(scene->mNumMeshes);

    // Process all meshes to get vertices AND bone weights
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];

        // 1. Process Standard Geometry (Vertices, Indices, Textures)
        // Reuse your existing processMesh logic!
        MeshData geoData = processMesh(mesh, scene);
        skeletalMesh->meshes.push_back(geoData);

        // 2. Process Bone Weights
        processSkeletalMesh(mesh, scene, *skeletalMesh, i);
    }

    if (scene->HasAnimations())
    {
        for (unsigned int i = 0; i < scene->mNumAnimations; i++)
        {
            aiAnimation* animation = scene->mAnimations[i];
            Animation newAnimation(animation, scene);
            skeletalMesh->animations[newAnimation.GetName()] = newAnimation;
        }
    }

    return skeletalMesh;
}

void AssetManager::processSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData& data, int meshIndex)
{
    // Initialize bone data for every vertex in this mesh with default -1/0 values
    data.bonesPerMesh[meshIndex].resize(mesh->mNumVertices);

    // Iterate through the bones provided by Assimp for this specific mesh
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        int boneID = -1;
        std::string boneName = mesh->mBones[i]->mName.C_Str();

        // If bone doesn't exist in our global map yet, add it
        if (data.boneInfoMap.find(boneName) == data.boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = data.boneCount;
            newBoneInfo.offset = AssimpToGLM(mesh->mBones[i]->mOffsetMatrix);

            data.boneInfoMap[boneName] = newBoneInfo;
            boneID = data.boneCount;
            data.boneCount++;
        }
        else {
            boneID = data.boneInfoMap[boneName].id;
        }

        // Get the weights for this bone
        auto weights = mesh->mBones[i]->mWeights;
        int numWeights = mesh->mBones[i]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; weightIndex++) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            // Add to our parallel array
            data.bonesPerMesh[meshIndex][vertexId].AddBoneData(boneID, weight);
        }
    }
}

void AssetManager::ExtractBoneWeightForVertices(std::vector<VertexBoneData>& vertices, aiMesh* mesh, const aiScene* scene, SkeletalMeshData& data)
{
}

void AssetManager::shutdown() {
    
}

    std::shared_ptr<StaticMeshData> AssetManager::importFBX(const std::string& path) {
        // cache check
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            if (auto cached = cache[path].lock())
                return cached;
        }

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_GenNormals
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Assimp failed: " + std::string(importer.GetErrorString()));
        }

        auto staticMesh = std::make_shared<StaticMeshData>();

		staticMesh->path = path;

        processNode(scene->mRootNode, scene, *staticMesh.get());

        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            cache[path] = staticMesh;
        }

        return staticMesh;
    }

    void AssetManager::processNode(aiNode* node, const aiScene* scene, StaticMeshData& staticMesh) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshData meshData = processMesh(mesh, scene);

			staticMesh.meshes.push_back(meshData);

            //staticMesh.MaterialId.push_back(node->mMeshes[i]);
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, staticMesh);
        }
    }

    MeshData AssetManager::processMesh(aiMesh* mesh, const aiScene* scene) {
        MeshData data;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }
            else {
                vertex.TexCoords = { 0.0f, 0.0f };
            }

            if (mesh->mTangents)
                vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
            if (mesh->mBitangents)
                vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };

            data.vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                data.indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            loadTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", data.textures);
            loadTextures(material, aiTextureType_SPECULAR, "texture_specular", data.textures);
        }

        return data;
    }

    void AssetManager::loadTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<TextureData>& textures) {
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            std::string texturePath = str.C_Str();

            this->loadTexture(texturePath);

            TextureData texMetadata;
            texMetadata.path = texturePath; // The mesh only needs to know the path
            texMetadata.type = typeName;
            texMetadata.width = 0;  // Metadata only
            texMetadata.height = 0; // Metadata only

            textures.push_back(texMetadata);
        }
    }