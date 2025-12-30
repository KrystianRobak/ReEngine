#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "ReflectionMacros.h"
#include "Collision/AABB.h"

REFCOMPONENT()
struct SphereCollider {
    REFVARIABLE()
        float radius = 0.5f;

    REFVARIABLE()
        glm::vec3 centerOffset = glm::vec3(0.0f);

    std::shared_ptr<AABB> aabb;

    SphereCollider() { aabb = std::make_shared<AABB>(); }

    // Updates the broadphase AABB based on position + radius
    void UpdateAABB(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        // Spheres are rotation invariant, just scale the radius
        float maxScale = std::max(scale.x, std::max(scale.y, scale.z));
        float worldRadius = radius * maxScale;

        // Calculate world center
        glm::vec3 worldCenter = pos + (rot * (centerOffset * scale));

        aabb->SetLocalBounds(glm::vec3(-worldRadius), glm::vec3(worldRadius));
        aabb->UpdateWorldTransform(worldCenter, glm::quat(1, 0, 0, 0), glm::vec3(1.0f));
    }
};