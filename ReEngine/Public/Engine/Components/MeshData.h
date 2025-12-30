#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "TextureData.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

// Pure CPU Data Container
struct MeshData {
    MeshData() = default;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // AABB for this specific submesh (useful for precise culling)
    glm::vec3 aabbMin{ 0.0f };
    glm::vec3 aabbMax{ 0.0f };

    // Material slot index for this submesh
    int materialIndex = 0;
};