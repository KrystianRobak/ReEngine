#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>



struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct TextureData {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> pixels;
    std::string type;
    std::string path;
};

struct MeshData {

	MeshData() = default;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureData> textures;

	bool isSetup = false;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
};
