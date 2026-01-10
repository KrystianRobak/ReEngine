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

    // --- Debug Data ---
    bool ShowDebugSkeleton = false;
    std::vector<float> DebugBoneLines;

    // --- Playback State ---
    std::string CurrentAnimationName;
    float CurrentTime = 0.0f;
    float AnimationSpeed = 1.0f;
    bool IsLooping = true;

    // --- NEW: Blending State (Managed by Animator) ---
    // We store the name of the anim we played last frame to detect changes
    std::string LastAnimationName;

    // The time the *previous* animation was at when we switched
    float HaltTime = 0.0f;

    // The time we updated to last frame (used to recover HaltTime after StateMachine resets CurrentTime)
    float LastFrameTime = 0.0f;

    bool IsBlending = false;
    float BlendTimer = 0.0f;
    float BlendDuration = 0.2f; // Default blend time (0.2s)
};