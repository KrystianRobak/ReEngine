#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


std::future<std::shared_ptr<StaticMeshData>> AssetManager::loadFBX(const std::string& path) {
    return pool->submit([this, path]() {
		auto mesh = this->importFBX(path);

        return mesh;
        });
}

std::future<std::shared_ptr<TextureData>> AssetManager::loadTexture(const std::string& path)
{
    return pool->submit([this, path]() {
        // cache check
        {
            std::lock_guard<std::mutex> lock(textureCacheMutex);
            if (auto cached = textureCache[path].lock())
                return cached;
        }
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load texture: " + path);
        }
		auto textureData = std::make_shared<TextureData>(width, height, channels, data);
        {
            std::lock_guard<std::mutex> lock(textureCacheMutex);
            textureCache[path] = textureData;
        }
        return textureData;
		});
}

void AssetManager::unloadTexture(const std::string& path)
{
    std::lock_guard<std::mutex> lock(textureCacheMutex);
	textureCache.erase(path);
}

void AssetManager::unloadMesh(const std::string& path)
{
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

            staticMesh.MaterialId.push_back(node->mMeshes[i]);
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
            TextureData tex;
            tex.path = str.C_Str();
            tex.type = typeName;

            int w, h, ch;
            unsigned char* data = stbi_load(tex.path.c_str(), &w, &h, &ch, 0);
            if (data) {
                tex.width = w;
                tex.height = h;
                tex.channels = ch;
                tex.pixels.assign(data, data + (w * h * ch));
                stbi_image_free(data);
            }

            textures.push_back(tex);
        }
    }