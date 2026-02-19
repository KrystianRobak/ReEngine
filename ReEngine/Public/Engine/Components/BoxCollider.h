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

    bool isTrigger = false;

    BoxCollider() { aabb = std::make_shared<AABB>(); }

    void FitToMesh(const glm::vec3& min, const glm::vec3& max) {
        if (!aabb) {
            aabb = std::make_shared<AABB>();
        }

        glm::vec3 newSize = max - min;

        if (glm::length(newSize) < 0.001f) {
            std::cout << "[Warning] FitToMesh received invalid bounds. Defaulting to 1.0." << std::endl;
            newSize = glm::vec3(1.0f);
            offset = glm::vec3(0.0f);
        }
        else {
            offset = (min + max) * 0.5f;
        }

        size = newSize;
        HasFittedToMesh = true;

        aabb->SetLocalBounds(size * -0.5f, size * 0.5f);
    }

    void Update(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        glm::vec3 finalPos = pos + (rot * (offset * scale));
        if (!HasFittedToMesh) aabb->SetLocalBounds(size * -0.5f, size * 0.5f);
        aabb->UpdateWorldTransform(finalPos, rot, scale);
    }
};