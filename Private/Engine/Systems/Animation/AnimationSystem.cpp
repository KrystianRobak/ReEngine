//
// Created by ragberr on 21.01.2025.
//

#include "Public/Engine/Systems/Animation/AnimationSystem.h"

#include "Engine/Components/Animated.h"
#include "Engine/Components/Transform.h"
#include "Engine/Core/Coordinator/Coordinator.h"

void AnimationSystem::Update(float dt) {
    Step(dt);
}

void AnimationSystem::SetEntityTransform(unsigned entity, glm::vec3 Position, glm::vec3 Rotation) {
    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    auto& transform = coordinator->GetComponent<Transform>(entity);
    transform.position = Position;
    transform.rotation = glm::quat(glm::radians(Rotation));
}

void AnimationSystem::Step(float t) {
    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    for (auto entity : mEntities) {
        auto& animated = coordinator->GetComponent<Animated>(entity);

        // Skip if no animations or keyframes
        if (animated.AnimPath.Animations.empty()) continue;

        auto& currentAnimation = animated.AnimPath.Animations[0];

        // Find the appropriate keyframes for interpolation
        auto keyframeIt = std::lower_bound(
            currentAnimation.keyframes.begin(),
            currentAnimation.keyframes.end(),
            CurrentFrame,
            [](const Keyframe& kf, int frame) { return kf.time < frame; }
        );

        // If we're before the first keyframe or after the last, handle edge cases
        if (keyframeIt == currentAnimation.keyframes.begin() ||
            keyframeIt == currentAnimation.keyframes.end()) {
            continue;
        }

        // Get previous and next keyframes
        auto prevKeyframe = std::prev(keyframeIt);
        auto nextKeyframe = keyframeIt;


        // Interpolate based on selected method
        glm::vec3 interpolatedPosition, interpolatedRotation;
        switch (currentAnimation.interpolation) {
            case AnimationInterpolation::Linear:
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t);
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t);
                break;

            case AnimationInterpolation::Cubic:
                // Implement cubic interpolation (you might want to use glm's cubic interpolation)
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t * t * (3.0f - 2.0f * t));
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t * t * (3.0f - 2.0f * t));
                break;

            case AnimationInterpolation::Quartic:
                interpolatedPosition = glm::mix(prevKeyframe->position, nextKeyframe->position, t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f));
                interpolatedRotation = glm::mix(prevKeyframe->rotation, nextKeyframe->rotation, t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f));
                break;
        }

        // Update the entity's transform
        // Note: You'll need to add a method to actually set the transform
        SetEntityTransform(entity, interpolatedPosition, interpolatedRotation);

        // Update current keyframe
        currentAnimation.CurrentKeyFrame = *nextKeyframe;
    }

    // Increment frame counter
    CurrentFrame++;
}

void AnimationSystem::Init() {
    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
}
