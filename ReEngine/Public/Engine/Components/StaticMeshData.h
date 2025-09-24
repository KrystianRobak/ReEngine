#pragma once

#include "MeshData.h"

struct StaticMeshData {

	StaticMeshData() : aabbMin(glm::vec3(FLT_MAX)), aabbMax(glm::vec3(-FLT_MAX)) {}

    std::vector<int> MaterialId;
    std::vector<MeshData> meshes;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};
