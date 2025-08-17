//
// Created by ragberr on 21.01.2025.
//

#include "Engine/Systems/Animation/AnimationSystem.h"

#include "Engine/Components/Animated.h"
#include "Engine/Components/Transform.h"
#include "Engine/Core/Coordinator/Coordinator.h"

void AnimationSystem::Update(float dt) {
    Step(dt);
}

void AnimationSystem::SetEntityTransform(unsigned entity, glm::vec3 Position, glm::vec3 Rotation) {
  /*  std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    auto& transform = coordinator->GetComponent<Transform>(entity);
    transform.position = Position;
    transform.rotation = glm::quat(glm::radians(Rotation));*/
}

void AnimationSystem::Step(float dt) {
   /* std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    for (auto entity : mEntities) {
        auto& animated = coordinator->GetComponent<Animated>(entity);

        if (animated.AnimPath.Animations.empty()) continue;

        auto& currentAnimation = animated.AnimPath.Animations[0];
        animated.AnimPath.CurrentTime++;

        if (animated.AnimPath.CurrentTime < animated.AnimPath.BeginTime ||
            animated.AnimPath.CurrentTime > animated.AnimPath.EndTime) {
            continue;
        }

        auto keyframeIt = std::lower_bound(
            currentAnimation.keyframes.begin(),
            currentAnimation.keyframes.end(),
            CurrentFrame,
            [](const Keyframe& kf, int frame) { return kf.time < frame; }
        );

        if (keyframeIt == currentAnimation.keyframes.begin()) {
            SetEntityTransform(entity, keyframeIt->position, keyframeIt->rotation);
            continue;
        }
        if (keyframeIt == currentAnimation.keyframes.end()) {
            SetEntityTransform(entity, std::prev(keyframeIt)->position, std::prev(keyframeIt)->rotation);
            continue;
        }


        auto prevKeyframe = std::prev(keyframeIt);
        auto nextKeyframe = keyframeIt;


        float keyframeDelta = static_cast<float>(nextKeyframe->time - prevKeyframe->time);
        float t = static_cast<float>(CurrentFrame - prevKeyframe->time) / keyframeDelta;


        glm::vec3 interpolatedPosition = glm::vec3(), interpolatedRotation = glm::vec3();
        switch (static_cast<AnimationInterpolation>(currentAnimation.CurrentKeyFrame.interpolation)) {
            case AnimationInterpolation::Linear:
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t);
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t);
                break;
            case AnimationInterpolation::Cubic:
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t * t * (3.0f - 2.0f * t));
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t * t * (3.0f - 2.0f * t));
                break;
            case AnimationInterpolation::Quartic:
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f));
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f));
                break;
        }

        SetEntityTransform(entity, interpolatedPosition, interpolatedRotation);


        currentAnimation.CurrentKeyFrame = *nextKeyframe;
    }
    LastFrame = CurrentFrame;
    CurrentFrame++;*/
}

void AnimationSystem::Init() {
  /*  std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    coordinator->AddEventListener(METHOD_LISTENER_NO_PARAM(Events::Application::TOGGLE, AnimationSystem::Reset));*/
}

void AnimationSystem::Reset() {
    //std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

    //for (auto entity : mEntities) {
    //    auto& animated = coordinator->GetComponent<Animated>(entity);
    //    animated.AnimPath.CurrentTime = 0;
    //}
}
