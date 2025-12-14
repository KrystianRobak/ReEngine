#pragma once
#include <glm/glm.hpp>
#include "ReflectionMacros.h"

// Define a simple structure for physical properties
REFCOMPONENT()
struct REFLECTION_API RigidBody {
    REFVARIABLE() glm::vec3 velocity = glm::vec3(0.0f);
    REFVARIABLE() glm::vec3 acceleration = glm::vec3(0.0f);
    REFVARIABLE() float mass = 1.0f;
    REFVARIABLE() float friction = 0.5f;     // 0 = ice, 1 = sandpaper
    REFVARIABLE() float restitution = 0.0f;  // Bounciness: 0 = no bounce, 1 = super ball
    REFVARIABLE() bool useGravity = true;
    REFVARIABLE() bool isStatic = false;     // True for floors/walls
};