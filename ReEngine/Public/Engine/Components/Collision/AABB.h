#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <limits>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

class AABB {
public:
    AABB() : localMin(-0.5f), localMax(0.5f) {
        UpdateWorldTransform(glm::vec3(0), glm::quat(1, 0, 0, 0), glm::vec3(1));
    }

    ~AABB() { cleanup(); }

    // Static resources
    static unsigned int staticVAO, staticVBO, staticEBO;
    static bool buffersInitialized;

    // --- Core API ---
    glm::vec3 GetMin() const { return worldMin; }
    glm::vec3 GetMax() const { return worldMax; }

    // Sets the raw size of the object (e.g., {-0.5, -0.5, -0.5} to {0.5, 0.5, 0.5} for a unit box)
    void SetLocalBounds(const glm::vec3& min, const glm::vec3& max) {
        localMin = min;
        localMax = max;
        // Recalculate immediately with current transform
        UpdateWorldTransform(currentPos, currentRot, currentScale);
    }

    void UpdateWorldTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        currentPos = pos;
        currentRot = rot;
        currentScale = scale;

        modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
            glm::mat4_cast(rot) *
            glm::scale(glm::mat4(1.0f), scale);

        // Recompute AABB in World Space (AABB rotation)
        // We take the 8 local corners, transform them, and find the new min/max
        std::vector<glm::vec3> corners = GetEightCornersLocal();
        worldMin = glm::vec3(std::numeric_limits<float>::max());
        worldMax = glm::vec3(std::numeric_limits<float>::lowest());

        for (const auto& corner : corners) {
            glm::vec4 worldPt = modelMatrix * glm::vec4(corner, 1.0f);
            worldMin = glm::min(worldMin, glm::vec3(worldPt));
            worldMax = glm::max(worldMax, glm::vec3(worldPt));
        }
    }

    void SetIsColliding(bool isColliding) { collides = isColliding; }

    // --- Debug Drawing ---
    void draw(Shader* shader) const;
    static void setupStaticBuffers();
    void cleanup() {};

private:
    glm::vec3 localMin;
    glm::vec3 localMax;

    glm::vec3 worldMin;
    glm::vec3 worldMax;

    // Cache for debug drawing
    glm::vec3 currentPos;
    glm::quat currentRot;
    glm::vec3 currentScale;
    glm::mat4 modelMatrix{ 1.0f };

    bool collides = false;

    std::vector<glm::vec3> GetEightCornersLocal() const {
        return {
            localMin,
            glm::vec3(localMax.x, localMin.y, localMin.z),
            glm::vec3(localMin.x, localMax.y, localMin.z),
            glm::vec3(localMax.x, localMax.y, localMin.z),
            glm::vec3(localMin.x, localMin.y, localMax.z),
            glm::vec3(localMax.x, localMin.y, localMax.z),
            glm::vec3(localMin.x, localMax.y, localMax.z),
            localMax
        };
    }
};