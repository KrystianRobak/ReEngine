#pragma once

#include "MeshData.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Represents the raw data loaded from disk (CPU side only)
struct StaticMeshData {
    StaticMeshData() = default;

    std::vector<MeshData> meshes;
    std::string path;

    // AABB for the entire model (used for Frustum Culling)
    glm::vec3 aabbMin{ 0.0f };
    glm::vec3 aabbMax{ 0.0f };
};