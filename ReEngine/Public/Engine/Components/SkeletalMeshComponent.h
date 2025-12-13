#pragma once
#include "ReflectionMacros.h"
#include "SkeletalMeshData.h"
#include <vector>
#include <memory>

REFCOMPONENT()
struct SkeletalMeshComponent
{
    REFVARIABLE()
        std::string AssetPath;

    REFVARIABLE()
        int MaterialId = -1;

    // Runtime data
    std::shared_ptr<SkeletalMeshData> MeshResource;

    // The final matrices sent to the shader this frame
    std::vector<glm::mat4> FinalBoneMatrices;

    // Animation State
    float CurrentTime = 0.0f;
    float AnimationSpeed = 1.0f;
    
    REFVARIABLE()
    std::string CurrentAnimationName;

    // TODO: Add reference to an AnimationClip resource here
};