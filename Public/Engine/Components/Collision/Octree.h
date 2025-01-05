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
        glm::vec3 nodeMin = center - glm::vec3(size/2);
        glm::vec3 nodeMax = center + glm::vec3(size/2);
        return !(aabb.GetMin().x > nodeMax.x || aabb.GetMax().x < nodeMin.x ||
                aabb.GetMin().y > nodeMax.y || aabb.GetMax().y < nodeMin.y ||
                aabb.GetMin().z > nodeMax.z || aabb.GetMax().z < nodeMin.z);
    }

    OctreeNode* getChildContainingPoint(const glm::vec3& point) {
        if (!hasChildren) return nullptr;

        for (auto& child : children) {
            glm::vec3 childMin = child->center - glm::vec3(child->size/2);
            glm::vec3 childMax = child->center + glm::vec3(child->size/2);

            if (point.x >= childMin.x && point.x <= childMax.x &&
                point.y >= childMin.y && point.y <= childMax.y &&
                point.z >= childMin.z && point.z <= childMax.z) {
                return child.get();
            }
        }
        return nullptr;
    }

    void subdivide() {
        float halfSize = size / 2;
        int index = 0;

        for (int x : {-1, 1}) {
            for (int y : {-1, 1}) {
                for (int z : {-1, 1}) {
                    glm::vec3 newCenter = center + glm::vec3(
                        x * halfSize/2,
                        y * halfSize/2,
                        z * halfSize/2
                    );
                    children[index] = std::make_unique<OctreeNode>(newCenter, halfSize);
                    index++;
                }
            }
        }
        hasChildren = true;
    }

public:
    OctreeNode(const glm::vec3& c, float s) : center(c), size(s) {}

    void insert(AABB* obj, int depth = 0) {
        if (hasChildren) {
            OctreeNode* child = getChildContainingPoint((obj->GetMin() + obj->GetMax()) * 0.5f);
            if (child && child->intersectsAABB(*obj)) {
                child->insert(obj, depth + 1);
                return;
            }
        }

        objects.push_back(obj);

        if (objects.size() > MAX_OBJECTS && depth < MAX_DEPTH) {
            if (!hasChildren) {
                subdivide();
            }

            auto objectsCopy = objects;
            objects.clear();
            for (auto* obj : objectsCopy) {
                insert(obj, depth);
            }
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
