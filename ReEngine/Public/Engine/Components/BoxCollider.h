#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "ReflectionMacros.h"
#include "Collision/AABB.h" 

REFCOMPONENT()
struct REFLECTION_API BoxCollider {
    // Offset from the entity's Transform position
    REFVARIABLE() glm::vec3 offset = glm::vec3(0.0f);
    // Size of the box
    REFVARIABLE() glm::vec3 size = glm::vec3(1.0f);

    // The runtime AABB instance
    std::shared_ptr<AABB> aabb;

    BoxCollider() {
        aabb = std::make_shared<AABB>();
    }
};