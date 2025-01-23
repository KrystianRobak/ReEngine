#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <limits>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/Systems/Render/Shader.h"

class AABB {
public:
    AABB() : minOffset(std::numeric_limits<float>::max()),
             maxOffset(std::numeric_limits<float>::lowest()) {
        setupStaticBuffers();
    }

    ~AABB() {
        cleanup();
    }

    glm::vec3 GetCentre()
    {
        return currentPosition;
    }

    // Static vertex and index data shared across all AABBs
    static unsigned int staticVAO, staticVBO, staticEBO;
    static bool buffersInitialized;

    glm::vec3 GetMin() const { return transformedMin; }
    glm::vec3 GetMax() const { return transformedMax; }

    void SetOffsets(const std::tuple<glm::vec3, glm::vec3>& MinMax) {
        glm::vec3 meshMin = std::get<0>(MinMax);
        glm::vec3 meshMax = std::get<1>(MinMax);

        // Calculate offsets relative to the current position
        minOffset = meshMin - currentPosition;
        maxOffset = meshMax - currentPosition;

        // Update transformed bounds to reflect new offsets
        updateTransformedBounds();
    }

    void SetIsColliding(bool isColliding) {
        this->collides = isColliding;
    }

    void updatePosition(const glm::vec3& newPosition) {
        currentPosition = newPosition;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, newPosition);
        this->modelMatrix = model;

        updateTransformedBounds();
    }

    void draw(Shader* shader) const {
        //if (!buffersInitialized) {
        //    std::cout << "Buffers not initialized!" << std::endl;
        //    return;
        //};
        std::vector<unsigned int> indices = {
            0, 1, 1, 3, 3, 2, 2, 0,
            4, 5, 5, 7, 7, 6, 6, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        glGenVertexArrays(1, &staticVAO);
        glGenBuffers(1, &staticVBO);
        glGenBuffers(1, &staticEBO);

        glBindVertexArray(staticVAO);

        glBindBuffer(GL_ARRAY_BUFFER, staticVBO);
        glBufferData(GL_ARRAY_BUFFER, corners.size() * sizeof(glm::vec3), corners.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(!collides) {
            shader->SetVec3("color", {1.0f, 0.0f, 0.0f});
        }
        else {
            shader->SetVec3("color", {0.0f, 1.0f, 0.0f});
        }

        shader->SetMat4("Model", modelMatrix);

        glBindVertexArray(staticVAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void updateTransformedBounds() {
        this->corners = {
            currentPosition + minOffset,
            currentPosition + glm::vec3(maxOffset.x, minOffset.y, minOffset.z),
            currentPosition + glm::vec3(minOffset.x, maxOffset.y, minOffset.z),
            currentPosition + glm::vec3(maxOffset.x, maxOffset.y, minOffset.z),
            currentPosition + glm::vec3(minOffset.x, minOffset.y, maxOffset.z),
            currentPosition + glm::vec3(maxOffset.x, minOffset.y, maxOffset.z),
            currentPosition + glm::vec3(minOffset.x, maxOffset.y, maxOffset.z),
            currentPosition + maxOffset
        };

        transformedMin = glm::vec3(std::numeric_limits<float>::max());
        transformedMax = glm::vec3(std::numeric_limits<float>::lowest());

        for (const auto& corner : corners) {
            glm::vec4 transformed = modelMatrix * glm::vec4(corner, 1.0f);
            glm::vec3 transformedCorner(transformed);
            transformedMin = glm::min(transformedMin, transformedCorner);
            transformedMax = glm::max(transformedMax, transformedCorner);
        }
    }

private:
    glm::vec3 minOffset;
    glm::vec3 maxOffset;
    glm::vec3 currentPosition;
    glm::vec3 transformedMin;
    glm::vec3 transformedMax;
    glm::mat4 modelMatrix;
    std::vector<glm::vec3> corners;
    bool collides;

    static void setupStaticBuffers() {
        if (buffersInitialized) return;

        buffersInitialized = true;
        std::cout << "Initialized Buffers" << std::endl;
    }

    static void cleanup() {
        if (buffersInitialized) {
            glDeleteVertexArrays(1, &staticVAO);
            glDeleteBuffers(1, &staticVBO);
            glDeleteBuffers(1, &staticEBO);
            buffersInitialized = false;
        }
    }
};
