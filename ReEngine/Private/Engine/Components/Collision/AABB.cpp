#include "Engine/Components/Collision/AABB.h"
#include <iostream>

unsigned int AABB::staticVAO = 0;
unsigned int AABB::staticVBO = 0;
unsigned int AABB::staticEBO = 0;
bool AABB::buffersInitialized = false;

void AABB::setupStaticBuffers() {
    if (buffersInitialized) return;

    std::vector<unsigned int> indices = {
        0, 1, 1, 3, 3, 2, 2, 0, // Bottom
        4, 5, 5, 7, 7, 6, 6, 4, // Top
        0, 4, 1, 5, 2, 6, 3, 7  // Sides
    };

    glGenVertexArrays(1, &staticVAO);
    glGenBuffers(1, &staticVBO);
    glGenBuffers(1, &staticEBO);

    glBindVertexArray(staticVAO);

    glBindBuffer(GL_ARRAY_BUFFER, staticVBO);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
    buffersInitialized = true;
}

void AABB::draw(Shader* shader) const {
    if (!buffersInitialized) const_cast<AABB*>(this)->setupStaticBuffers();
    if (!shader) return;

    // Upload current local corners for this specific shape
    std::vector<glm::vec3> corners = GetEightCornersLocal();
    glBindVertexArray(staticVAO);
    glBindBuffer(GL_ARRAY_BUFFER, staticVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, corners.size() * sizeof(glm::vec3), corners.data());

    shader->SetVec3("color", collides ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
    shader->SetMat4("Model", modelMatrix);

    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}