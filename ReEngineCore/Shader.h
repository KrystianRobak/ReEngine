#pragma once

#include "CoreExport.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>
#include <iostream>

class CORE_API Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const char* vertexCode, const char* fragmentCode, bool IsCode);

    void ChangeShaderDefineStatus(uint32_t amount);
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void Use();

    unsigned int get_program_id() { return ID; }


    // utility uniform functions

    void SetBool(const std::string& name, bool value) const;
    // ------------------------------------------------------------------------
    void SetInt(const std::string& name, int value) const;

    // ------------------------------------------------------------------------
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, glm::vec2 value) const;
    void SetVec3(const std::string& name, glm::vec3 value) const;
    void SetVec4(const std::string& name, glm::vec4 value) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
private:

    std::string ReadFile(const std::string& path);
    void CheckCompileErrors(unsigned int shader, std::string type);

private:
	unsigned int ID;
    const char* mFragmentPath;
};

