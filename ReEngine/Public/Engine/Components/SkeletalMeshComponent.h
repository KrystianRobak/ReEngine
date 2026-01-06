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

    // The visual asset
    std::shared_ptr<MeshResource> MeshResource; //

    // The output for the shader
    std::vector<glm::mat4> FinalBoneMatrices;   //

    // Playback "Registers" (The Brain writes to these)
    std::string CurrentAnimationName;           //
    float CurrentTime = 0.0f;                   //
    float AnimationSpeed = 1.0f;                //
    bool IsLooping = true;
};