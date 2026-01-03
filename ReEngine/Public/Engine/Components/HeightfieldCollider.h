#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <cmath>
#include "ReflectionMacros.h"
#include "Collision/AABB.h"

REFCOMPONENT()
struct HeightfieldCollider {
    // Parameters stored from WorldGenSystem
    REFVARIABLE() int width = 0;
    REFVARIABLE() int depth = 0;
    REFVARIABLE() float scale = 0.0f;
    REFVARIABLE() float heightMultiplier = 0.0f;

    REFVARIABLE() float halfWidth = 0.0f;
    REFVARIABLE() float halfDepth = 0.0f;

    std::shared_ptr<AABB> aabb;

    HeightfieldCollider() { aabb = std::make_shared<AABB>(); }

    // The single source of truth for terrain height
    float GetHeightAt(float x, float z) const {
        // Same math as WorldGenSystem
        float y = std::sin(x * scale) * std::cos(z * scale);
        y += std::sin(x * scale * 2.5f) * std::cos(z * scale * 2.5f) * 0.5f;
        return y * heightMultiplier;
    }

    void Update(const glm::vec3& pos) {
        // Create an AABB that encompasses the whole terrain
        // Since the terrain is centered at (0,0), min is (-halfWidth, -height, -halfDepth)

        // Approximate max height based on multiplier (1.0 + 0.5 = 1.5 max amplitude)
        float maxH = heightMultiplier * 1.5f;

        glm::vec3 min(-halfWidth, -maxH + pos.y, -halfDepth);
        glm::vec3 max(halfWidth, maxH + pos.y, halfDepth);

        aabb->SetLocalBounds(min, max);
        // Terrain is usually static, so rotation/scale in AABB update might be ignored or identity
        aabb->UpdateWorldTransform(pos, glm::quat(1, 0, 0, 0), glm::vec3(1.0f));
    }
};