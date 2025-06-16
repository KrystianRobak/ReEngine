#include "Engine/Components/Collision/Octree.h"

bool checkCollision(const AABB& a, const AABB& b) {
    return !(a.GetMin().x > b.GetMax().x || a.GetMax().x < b.GetMin().x ||
        a.GetMin().y > b.GetMax().y || a.GetMax().y < b.GetMin().y ||
        a.GetMin().z > b.GetMax().z || a.GetMax().z < b.GetMin().z);
}

std::vector<std::pair<AABB*, AABB*>> checkCollisions(std::vector<AABB*>& objects) {
    // Determine appropriate root size
    glm::vec3 minBound(std::numeric_limits<float>::max());
    glm::vec3 maxBound(std::numeric_limits<float>::lowest());

    for (auto* obj : objects) {
        minBound = glm::min(minBound, obj->GetMin());
        maxBound = glm::max(maxBound, obj->GetMax());
    }

    // Calculate center and size for the root node
    glm::vec3 center = (minBound + maxBound) * 0.5f;
    float size = glm::length(maxBound - minBound) * 0.6f; // Add some margin

    OctreeNode root(center, size);
    std::vector<std::pair<AABB*, AABB*>> collisions;

    // Insert all objects
    for (auto* obj : objects) {
        root.insert(obj);
    }

    // Check for collisions
    for (size_t i = 0; i < objects.size(); i++) {
        auto* obj = objects[i];
        std::vector<AABB*> nearby;
        root.queryRegion(*obj, nearby);

        for (auto* other : nearby) {
            // Skip if it's the same object or we've already checked this pair
            if (other == obj || other < obj) {
                continue;
            }

            if (checkCollision(*obj, *other)) {
                // Set collision flags on both objects
                obj->SetIsColliding(true);
                other->SetIsColliding(true);
                collisions.emplace_back(obj, other);
            }
        }
    }

    return collisions;
}