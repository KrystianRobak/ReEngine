#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "ReflectionMacros.h"
#include "Collision/AABB.h" 

REFCOMPONENT()
struct BoxCollider {
    REFVARIABLE() glm::vec3 offset = glm::vec3(0.0f);
    REFVARIABLE() glm::vec3 size = glm::vec3(1.0f);

    bool HasFittedToMesh = false;
    std::shared_ptr<AABB> aabb;

    BoxCollider() { aabb = std::make_shared<AABB>(); }

    void FitToMesh(const glm::vec3& min, const glm::vec3& max) {
        if (!aabb)
        {
            aabb = std::make_shared<AABB>();
        }
        size = max - min;
        offset = (min + max) * 0.5f;
        HasFittedToMesh = true;
        // Set Local AABB centered at 0, sized to match mesh
        aabb->SetLocalBounds(size * -0.5f, size * 0.5f);
    }

    void Update(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        // Apply local offset in world space
        glm::vec3 finalPos = pos + (rot * (offset * scale));

        // If not auto-fitted, ensure AABB matches manual size
        if (!HasFittedToMesh) aabb->SetLocalBounds(size * -0.5f, size * 0.5f);

        aabb->UpdateWorldTransform(finalPos, rot, scale);
    }
};