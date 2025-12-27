#include "Shader.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// 1. HELPER: Finds the actual path of the shader
std::string ResolveShaderPath(const std::string& path) {
    // Check if path is already valid
    if (fs::exists(path)) return path;

    // Search common relative locations (e.g., up two levels for dev builds)
    fs::path searchPaths[] = {
        fs::current_path() / path,
        fs::current_path() / "shaders" / path,
        fs::current_path().parent_path() / path,
        fs::current_path().parent_path().parent_path() / path // Useful for IDE builds
    };

    for (const auto& p : searchPaths) {
        if (fs::exists(p)) return p.string();
    }

    return ""; // Not found
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    try {
        std::string vCode = ReadFile(vertexPath);
        std::string fCode = ReadFile(fragmentPath);

        const char* vShaderCode = vCode.c_str();
        const char* fShaderCode = fCode.c_str();

        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        CheckCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        std::cout << "Successfully loaded: " << vertexPath << " and " << fragmentPath << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL SHADER ERROR: " << e.what() << std::endl;
    }
}

Shader::Shader(const char* vertexCode, const char* fragmentCode, bool IsCode)
{
    // 1. Compile shaders
    unsigned int vertex, fragment;

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, NULL);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode, NULL);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

    // Delete shaders (no longer needed after linking)
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    std::cout << "SHADER::COMPILED_SUCCESSFULLY" << std::endl;
}


void Shader::ChangeShaderDefineStatus(uint32_t amount)
{
    std::filesystem::path filePath = mFragmentPath;

    // Open the file for reading
    std::ifstream FileToChange(filePath, std::ios::in);

    if (!FileToChange.is_open())
    {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return;
    }

    std::vector<std::string> fileLines;
    std::string line;

    // Read lines from the file
    while (std::getline(FileToChange, line))
    {
        // Remove any trailing '\r' (for Windows line endings on Linux)
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        fileLines.push_back(line);
    }

    FileToChange.close();

    // Check if the file has enough lines
    if (fileLines.size() < 2)
    {
        std::cerr << "File does not have enough lines to replace the second line." << std::endl;
        return;
    }

    // Replace the second line with the new define statement
    fileLines[2] = "#define LightArraySize " + std::to_string(amount);

    // Open the file for writing (overwrites the entire file)
    std::ofstream FileToWrite(filePath, std::ios::out | std::ios::trunc);

    if (!FileToWrite.is_open())
    {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

    // Write all lines back to the file
    for (const auto& fileLine : fileLines)
    {
        FileToWrite << fileLine << "\n"; // Write with platform-independent newline
    }

    FileToWrite.close();
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    try {
        std::string vCode = ReadFile(vertexPath);
        std::string fCode = ReadFile(fragmentPath);

        const char* vShaderCode = vCode.c_str();
        const char* fShaderCode = fCode.c_str();

        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        CheckCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        std::cout << "Successfully loaded: " << vertexPath << " and " << fragmentPath << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL SHADER ERROR: " << e.what() << std::endl;
    }
}

void Shader::Use()
{
    glUseProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}


void Shader::SetVec2(const std::string& name, glm::vec2 value) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}


void Shader::SetVec3(const std::string& name, glm::vec3 value) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}


void Shader::SetVec4(const std::string& name, glm::vec4 value) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

std::string Shader::ReadFile(const std::string& path) {
    std::string resolved = ResolveShaderPath(path);
    if (resolved.empty()) {
        throw std::runtime_error("Shader file not found: " + path);
    }

    std::ifstream file(resolved, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open shader file: " + resolved);
    }

    // Efficiently read entire file into string
    std::ostringstream sstr;
    sstr << file.rdbuf();
    return sstr.str();
}

void Shader::CheckCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
