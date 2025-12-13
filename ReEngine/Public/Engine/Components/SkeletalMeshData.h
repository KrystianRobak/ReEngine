#pragma once
#include "MeshData.h"
#include "StaticMeshData.h"
#include <map>
#include "../Animation/Animation.h"

#define MAX_BONE_INFLUENCE 4

// Standard structure for passing bone data to shaders
struct VertexBoneData {
    int BoneIDs[MAX_BONE_INFLUENCE];
    float Weights[MAX_BONE_INFLUENCE];

    VertexBoneData() {
        memset(BoneIDs, -1, sizeof(BoneIDs));
        memset(Weights, 0, sizeof(Weights));
    }

    void AddBoneData(int boneID, float weight) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (BoneIDs[i] < 0) {
                BoneIDs[i] = boneID;
                Weights[i] = weight;
                return;
            }
        }
    }
};

struct BoneInfo {
    int id;
    glm::mat4 offset; // Inverse Bind Pose Matrix
};

struct SkeletalMeshData : public StaticMeshData {
    // Parallel array to meshes[i].vertices, holds bone weights
    std::vector<std::vector<VertexBoneData>> bonesPerMesh;

    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCount = 0;

    std::map<std::string, Animation> animations;

    // The root node of the Assimp scene hierarchy (needed for traversing animation)
    // You might need a custom Node struct if you don't want to store raw aiNode*
    // For now, let's assume we copy the hierarchy or re-use Assimp's logic during import.
};