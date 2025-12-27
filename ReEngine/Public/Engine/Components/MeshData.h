#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "TextureData.h"
#include <string>
#include <vector>



struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct MeshData {

	MeshData() = default;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureData> textures;

	bool isSetup = false;

    glm::vec3 aabbMin = glm::vec3(FLT_MAX);
    glm::vec3 aabbMax = glm::vec3(-FLT_MAX);

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
	GLuint BVAO = 0;
};
