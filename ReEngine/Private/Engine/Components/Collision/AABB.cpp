#include "Engine/Components/Collision/AABB.h"

unsigned int AABB::staticVAO = 0;
unsigned int AABB::staticVBO = 0;
unsigned int AABB::staticEBO = 0;
bool AABB::buffersInitialized = false;

void AABB::setupStaticBuffers() {
    if (buffersInitialized) return;

    // Indices for drawing the AABB as 12 lines
    std::vector<unsigned int> indices = {
        0, 1, 1, 3, 3, 2, 2, 0, // Bottom face
        4, 5, 5, 7, 7, 6, 6, 4, // Top face
        0, 4, 1, 5, 2, 6, 3, 7  // Connecting edges
    };

    // Generate buffers
    glGenVertexArrays(1, &staticVAO);
    glGenBuffers(1, &staticVBO);
    glGenBuffers(1, &staticEBO);

    glBindVertexArray(staticVAO);

    // We'll update the vertex buffer later in draw()
    glBindBuffer(GL_ARRAY_BUFFER, staticVBO);
    // Just allocate space - we'll update the data in draw()
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    // Set up index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
    buffersInitialized = true;
    std::cout << "Initialized AABB static buffers" << std::endl;
}

void AABB::cleanup() {
    if (buffersInitialized) {
        glDeleteVertexArrays(1, &staticVAO);
        glDeleteBuffers(1, &staticVBO);
        glDeleteBuffers(1, &staticEBO);
        buffersInitialized = false;
    }
}