#pragma once

#include "ReflectionMacros.h"
#include <AnimGraph.h>
#include <memory>
#include <unordered_map>

REFCOMPONENT()
struct StateMachine
{
    REFVARIABLE()
    std::string GraphAssetPath;

    // The loaded resource (Shared between all Orcs/Players)
    std::shared_ptr<AnimationGraphResource> GraphResource;

    // --- 2. The Runtime State ---
    int CurrentNodeID = -1;

    // The "Blackboard" - specific to THIS entity instance
    REFVARIABLE()
    std::unordered_map<std::string, AnimVar> Blackboard;

    // --- 3. The Interrupt Slot ---
    bool IsSlotPlaying = false;
    std::string SlotAnimName;
    float SlotTimeRemaining = 0.0f;
};