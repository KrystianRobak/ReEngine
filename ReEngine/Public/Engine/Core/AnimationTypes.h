//
// Created by ragberr on 21.01.2025.
//
#pragma once
#include <chrono>
#include <set>
#include <string>
#include <glm/vec3.hpp>

#include "imgui/imgui.h"
#include "Types.h"

enum class AnimationInterpolation {
    Linear = 0,
    Cubic = 1,
    Quartic = 2,
};

struct Keyframe {
    int interpolation;
    int time;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct Animation {
    Keyframe CurrentKeyFrame;
    std::vector<Keyframe> keyframes;
};

struct AnimationPath {
    float EndTime = 0.0f;
    float BeginTime = 0.0f;
    std::string Name;

    Entity AnimationTarget;
    float CurrentTime = 0.0f;
    float Duration = 0.0f;
    bool Selected = false;

    std::vector<Animation> Animations;

    AnimationPath(float EndTime, float BeginTime, std::string Name) : EndTime(EndTime), BeginTime(BeginTime), Name(Name) {
        this->Duration = EndTime - BeginTime;
    };

    AnimationPath() {
        this->Duration = EndTime - BeginTime;
        this->EndTime = 1.0f;
        this->BeginTime = 0.0f;
        this->Name = "NewTrack";
    };

    AnimationPath(std::string Name, Entity entity) : Name(Name) , AnimationTarget(entity) {
        this->Duration = EndTime - BeginTime;
        this->EndTime = 1.0f;
        this->BeginTime = 0.0f;
    };

};

struct ReSequencer {
    std::vector<AnimationPath*> AnimPaths;
    float timeline_start = 0.0f; // Start of the timeline
    float timeline_end = 3200.0f;  // End of the timeline
    float TimeInterval = 160.0f;

    float MaxTimeInterval = 10.f;
    float MinTimeInterval = 0.1f;
};
