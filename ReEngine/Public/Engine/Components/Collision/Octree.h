#pragma once

#include <vector>
#include <array>
#include <memory>
#include <glm/vec3.hpp>

#include "AABB.h"

bool checkCollision(const AABB& a, const AABB& b);
std::vector<std::pair<AABB*, AABB*>> checkCollisions(std::vector<AABB*>& objects);

class OctreeNode {
private:
    glm::vec3 center;
    float size;
    std::vector<AABB*> objects;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    static constexpr int MAX_OBJECTS = 8;
    static constexpr int MAX_DEPTH = 8;
    bool hasChildren = false;

    bool intersectsAABB(const AABB& aabb) const {

        glm::vec3 nodeMin = center - glm::vec3(size / 2);
        glm::vec3 nodeMax = center + glm::vec3(size / 2);

        return !(aabb.GetMax().x < nodeMin.x || aabb.GetMin().x > nodeMax.x ||
            aabb.GetMax().y < nodeMin.y || aabb.GetMin().y > nodeMax.y ||
            aabb.GetMax().z < nodeMin.z || aabb.GetMin().z > nodeMax.z);
    }

    int getOctantForPoint(const glm::vec3& point) const {
        int octant = 0;
        if (point.x >= center.x) octant |= 1;
        if (point.y >= center.y) octant |= 2;
        if (point.z >= center.z) octant |= 4;
        return octant;
    }

    OctreeNode* getChildContainingPoint(const glm::vec3& point) {
        if (!hasChildren) return nullptr;

        for (auto& child : children) {
            glm::vec3 childMin = child->center - glm::vec3(child->size / 2);
            glm::vec3 childMax = child->center + glm::vec3(child->size / 2);

            if (point.x >= childMin.x && point.x <= childMax.x &&
                point.y >= childMin.y && point.y <= childMax.y &&
                point.z >= childMin.z && point.z <= childMax.z) {
                return child.get();
            }
        }
        return nullptr;
    }

    void subdivide() {
        float childSize = size / 2.0f;
        float offset = childSize / 2.0f;

        for (int i = 0; i < 8; i++) {
            // Calculate new center based on octant index
            glm::vec3 newCenter = center;
            if (i & 1) newCenter.x += offset; else newCenter.x -= offset;
            if (i & 2) newCenter.y += offset; else newCenter.y -= offset;
            if (i & 4) newCenter.z += offset; else newCenter.z -= offset;

            children[i] = std::make_unique<OctreeNode>(newCenter, childSize);
        }
        hasChildren = true;
    }

public:
    OctreeNode(const glm::vec3& c, float s) : center(c), size(s) {}

    void insert(AABB* obj, int depth = 0) {
        // If the AABB doesn't intersect this node, don't insert
        if (!intersectsAABB(*obj)) {
            return;
        }

        // If we're at max depth or don't have too many objects yet, add to this node
        if (depth >= MAX_DEPTH || objects.size() < MAX_OBJECTS) {
            objects.push_back(obj);
            return;
        }

        // Create children if they don't exist
        if (!hasChildren) {
            subdivide();
        }

        // Try to insert into children
        bool insertedInChild = false;
        glm::vec3 objCenter = (obj->GetMin() + obj->GetMax()) * 0.5f;

        // First try the child that contains the center point
        int octant = getOctantForPoint(objCenter);
        if (children[octant]->intersectsAABB(*obj)) {
            children[octant]->insert(obj, depth + 1);
            insertedInChild = true;
        }

        // If we couldn't insert into the best child, try all others that intersect
        if (!insertedInChild) {
            // If the object spans multiple octants, add it to this node
            objects.push_back(obj);
        }
    }

    void queryRegion(const AABB& queryBox, std::vector<AABB*>& results) const {
        if (!intersectsAABB(queryBox)) return;

        results.insert(results.end(), objects.begin(), objects.end());

        if (hasChildren) {
            for (const auto& child : children) {
                child->queryRegion(queryBox, results);
            }
        }
    }
};