#include "../../../../Public/Engine/Components/Collision/Octree.h"

bool checkCollision(const AABB& a, const AABB& b) {
    return !(a.GetMin().x > b.GetMax().x || a.GetMax().x < b.GetMin().x ||
            a.GetMin().y > b.GetMax().y || a.GetMax().y < b.GetMin().y ||
            a.GetMin().z > b.GetMax().z || a.GetMax().z < b.GetMin().z);
}

std::vector<std::pair<AABB*, AABB*>> checkCollisions(std::vector<AABB*>& objects) {
    OctreeNode root(glm::vec3(0), 165.0f);
    std::vector<std::pair<AABB*, AABB*>> collisions;

    for (auto* obj : objects) {
        root.insert(obj);
    }

    for (auto* obj : objects) {
        std::vector<AABB*> nearby;
        root.queryRegion(*obj, nearby);

        for (auto* other : nearby) {
            if (other != obj && checkCollision(*obj, *other)) {
                collisions.emplace_back(obj, other);
            }
        }
    }

    return collisions;
}