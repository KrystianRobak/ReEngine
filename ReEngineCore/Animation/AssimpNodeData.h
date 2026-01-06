#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};