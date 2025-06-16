#pragma once
#include "Engine/Systems/Render/Shader.h"
#include "Mesh.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"


struct StaticMesh
{
public:
    StaticMesh(bool gamma = false) : gammaCorrection(gamma)
    {

    }
    void Draw(Shader& shader);

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    void loadModel(std::string const& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    std::tuple<glm::vec3, glm::vec3> getAABB();

private:
    bool AABBInitialized = false;
    std::tuple<glm::vec3, glm::vec3> AABBBounds;
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;
    bool gammaCorrection;
};

