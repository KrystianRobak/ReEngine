#pragma once
#include <glm/glm.hpp>
#include "ReflectionMacros.h"

REFCOMPONENT()
struct HeightfieldCollider {
    // Parameters stored from WorldGenSystem
    REFVARIABLE() int width = 0;
    REFVARIABLE() int depth = 0;
    REFVARIABLE() float scale = 0.0f;
    REFVARIABLE() float heightMultiplier = 0.0f;

    // The half-extents of the terrain, used to check if an object is within bounds
    REFVARIABLE() float halfWidth = 0.0f;
    REFVARIABLE() float halfDepth = 0.0f;
};