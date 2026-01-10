#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "TextureData.h"

// Pure CPU Data Container
struct MeshData {
    MeshData() = default;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;
    std::vector<glm::vec3> Tangents;
    std::vector<glm::vec3> Bitangents;
    std::vector<glm::ivec4> boneIDs;
    std::vector<glm::vec4> weights;

    std::vector<uint32_t> indices;

    // AABB for this specific submesh (useful for precise culling)
    glm::vec3 aabbMin{ 0.0f };
    glm::vec3 aabbMax{ 0.0f };

    // Material slot index for this submesh
    int materialIndex = 0;
};