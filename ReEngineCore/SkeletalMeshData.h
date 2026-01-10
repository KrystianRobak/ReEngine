#pragma once
#include "MeshData.h"
#include "StaticMeshData.h"
#include <map>
#include <vector>
#include <memory>
#include <future>
#include <atomic>
#include "ReTypes.h"

#define MAX_BONE_INFLUENCE 4

struct BoneProps
{
	std::string name;
	glm::mat4 offset;
};


struct SkeletalMeshData : public StaticMeshData {
    std::vector<BoneProps> boneInfoMap;
    int boneCount = 0;
};