#pragma once
#include "ReflectionMacros.h"
#include <vector>
#include <memory>
#include <string>

struct MeshResource;

REFCOMPONENT()
struct SkeletalMeshComponent
{
    REFVARIABLE()
        std::string AssetPath;

    REFVARIABLE()
        int MaterialId = -1;

    std::shared_ptr<MeshResource> MeshResource;
    std::vector<glm::mat4> FinalBoneMatrices;

    
    bool ShowDebugSkeleton = false;
    std::vector<float> DebugBoneLines;

    
    std::string CurrentAnimationName;
    float CurrentTime = 0.0f;

    REFVARIABLE()
    float AnimationSpeed = 1.0f;
    
    REFVARIABLE()
    bool IsLooping = true;

    
    
    std::string LastAnimationName;

    
    float HaltTime = 0.0f;

    
    float LastFrameTime = 0.0f;

    bool IsBlending = false;
    float BlendTimer = 0.0f;
    float BlendDuration = 0.2f; 
};

