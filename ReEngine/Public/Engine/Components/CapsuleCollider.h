#pragma once
#include "ReflectionMacros.h"
#include "Collision/AABB.h"

REFCOMPONENT()
struct CapsuleCollider {
    REFVARIABLE() float radius = 0.5f;
    REFVARIABLE() float height = 2.0f; // Total height
    REFVARIABLE() glm::vec3 offset = glm::vec3(0.0f);

    std::shared_ptr<AABB> aabb;
    CapsuleCollider() { aabb = std::make_shared<AABB>(); }

    void Update(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        float maxScale = std::max(scale.x, std::max(scale.y, scale.z));
        float r = radius * maxScale;
        float h = height * scale.y;

        // Approximate capsule AABB
        glm::vec3 min(-r, -h * 0.5f, -r);
        glm::vec3 max(r, h * 0.5f, r);

        aabb->SetLocalBounds(min, max);
        aabb->UpdateWorldTransform(pos + (rot * (offset * scale)), rot, scale);
    }
};