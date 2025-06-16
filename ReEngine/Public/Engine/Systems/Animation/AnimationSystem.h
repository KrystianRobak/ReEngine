//
// Created by ragberr on 21.01.2025.
#pragma once

#include "Engine/Components/Animated.h"
#include "Engine/Core/Coordinator/Coordinator.h"
#include "../System.h"
#include <memory>
class AnimationSystem : public System {
public:
    void Init();
    void Reset();
    void Update(float dt);
private:
    void SetEntityTransform(unsigned entity, glm::vec3 vec, glm::vec3 vec3);

    void Step(float t);


public:
    int CurrentFrame = 0;
    int LastFrame = 0;
};
