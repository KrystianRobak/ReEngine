#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <limits>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "Engine/Systems/Render/Shader.h"

class AABB {
public:
    AABB() : minOffset(std::numeric_limits<float>::max()),
        maxOffset(std::numeric_limits<float>::lowest()) {
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
        // Check if the mesh bounds are valid
        if (meshMin.x == meshMax.x && meshMin.y == meshMax.y && meshMin.z == meshMax.z) {
            std::cerr << "WARNING: Mesh bounds are invalid - min equals max!" << std::endl;
            // Set some default small box
            minOffset = glm::vec3(-0.5f);
            maxOffset = glm::vec3(0.5f);
        }
        else {
            // Calculate offsets relative to the current position
            minOffset = meshMin - currentPosition;
            maxOffset = meshMax - currentPosition;
        }
        // Update the corners and bounds with the new offsets
        updateTransformedBounds();
    }

    void SetIsColliding(bool isColliding) {
        this->collides = isColliding;
    }

    void updatePosition(const glm::vec3& newPosition, const glm::quat& newRotation, const glm::vec3& newScale) {
        currentPosition = newPosition;

        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), newPosition) *
            glm::mat4_cast(newRotation) *
            glm::scale(glm::mat4(1.0f), newScale);

        this->modelMatrix = model;
        // Now update the corners and transformed bounds
        updateTransformedBounds();

    }

    void draw(Shader* shader) const {
        if (!buffersInitialized) {
            std::cerr << "AABB buffers not initialized! Attempting to initialize now..." << std::endl;
            // Force initialization - cast away const to call the non-const method
            const_cast<AABB*>(this)->setupStaticBuffers();

            if (!buffersInitialized) {
                std::cerr << "Failed to initialize AABB buffers!" << std::endl;
                return;
            }
        }

        // Check if shader is valid
        if (!shader || shader->get_program_id() == 0) {
            std::cerr << "Invalid shader for AABB drawing!" << std::endl;
            return;
        }

        // Make sure we have valid corners data
        if (corners.size() != 8) {
            std::cerr << "Invalid AABB corners data! Expected 8 corners, got "
                << corners.size() << std::endl;
            return;
        }

        // Update the vertex buffer with current corner positions
        glBindVertexArray(staticVAO);
        glBindBuffer(GL_ARRAY_BUFFER, staticVBO);

        glBufferSubData(GL_ARRAY_BUFFER, 0, corners.size() * sizeof(glm::vec3), corners.data());

        // Set shader uniforms
        if (collides) {
            shader->SetVec3("color", { 0.0f, 1.0f, 0.0f }); // Green for collision
        }
        else {
            shader->SetVec3("color", { 1.0f, 0.0f, 0.0f }); // Red for no collision
        }

        shader->SetMat4("Model", modelMatrix);

        // Draw the AABB
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error in AABB::draw: " << err << std::endl;
        }

        glBindVertexArray(0);
    }

    void updateTransformedBounds() {
        // Clear the previous corners
        corners.clear();

        // Calculate all 8 corners of the AABB using the offsets and current position
        // Order: min corner, then permute X,Y,Z for other corners
        corners = {
            currentPosition + minOffset,                                              // 0: min point
            currentPosition + glm::vec3(maxOffset.x, minOffset.y, minOffset.z),       // 1: max X, min Y, min Z
            currentPosition + glm::vec3(minOffset.x, maxOffset.y, minOffset.z),       // 2: min X, max Y, min Z
            currentPosition + glm::vec3(maxOffset.x, maxOffset.y, minOffset.z),       // 3: max X, max Y, min Z
            currentPosition + glm::vec3(minOffset.x, minOffset.y, maxOffset.z),       // 4: min X, min Y, max Z
            currentPosition + glm::vec3(maxOffset.x, minOffset.y, maxOffset.z),       // 5: max X, min Y, max Z
            currentPosition + glm::vec3(minOffset.x, maxOffset.y, maxOffset.z),       // 6: min X, max Y, max Z
            currentPosition + maxOffset                                               // 7: max point
        };
        // Now calculate the transformed bounds for queries
        transformedMin = glm::vec3(std::numeric_limits<float>::max());
        transformedMax = glm::vec3(std::numeric_limits<float>::lowest());

        // Consider model transformation when calculating bounds
        for (const auto& corner : corners) {
            // Transform the corner by the model matrix
            glm::vec4 transformed = modelMatrix * glm::vec4(corner, 1.0f);
            glm::vec3 transformedCorner(transformed.x, transformed.y, transformed.z);

            // Update min/max bounds
            transformedMin = glm::min(transformedMin, transformedCorner);
            transformedMax = glm::max(transformedMax, transformedCorner);
        }
    }

    static void setupStaticBuffers();

    void cleanup();

private:
    glm::vec3 minOffset;
    glm::vec3 maxOffset;
    glm::vec3 currentPosition;
    glm::vec3 transformedMin;
    glm::vec3 transformedMax;
    glm::mat4 modelMatrix;
    std::vector<glm::vec3> corners;
    bool collides;

    

    
   
};