#include "Shader.h"

#include "Utils/FileUtils.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>


Shader::Shader()
{
}

Shader::Shader(const char* vertexCode, 
               const char* fragmentCode,
               const char* tesCtrlCode,
               const char* tesEvalCode,
               const char* geometryCode) : m_id(0)
{
    GLuint vShader;
    GLuint fShader;
    GLuint tcShader = -1;
    GLuint teShader = -1;
    GLuint gShader = -1;
    if (!CompileShader(GL_VERTEX_SHADER, vertexCode, vShader)) 
        return;
    if (!CompileShader(GL_FRAGMENT_SHADER, fragmentCode, fShader)) 
        return;
    if ((tesCtrlCode && tesEvalCode)) {
        CompileShader(GL_TESS_CONTROL_SHADER, tesCtrlCode, tcShader);
        CompileShader(GL_TESS_EVALUATION_SHADER, tesEvalCode, teShader);
    }
    if (geometryCode) {
        CompileShader(GL_GEOMETRY_SHADER, geometryCode, gShader);
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vShader);
    glAttachShader(m_id, fShader);
    if (tcShader != -1 && teShader != -1) {
        glAttachShader(m_id, tcShader);
        glAttachShader(m_id, teShader);
    }
    if (gShader != -1) {
        glAttachShader(m_id, gShader);
    }

    GLint success;
    glLinkProgram(m_id);
    glGetProgramiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
        
        std::vector<char> log(length);
        glGetProgramInfoLog(fShader, length, &length, log.data());
        fprintf(stderr, "ERROR: Program linking failed with message :\n%s\n", log.data());

        m_id = 0;
        return;
    }

    // Cleanup
    glDetachShader(m_id, vShader);
    glDetachShader(m_id, fShader);
    if (tcShader != -1 && teShader != -1) {
        glDetachShader(m_id, tcShader);
        glDetachShader(m_id, teShader);
    }
    if (gShader != -1) {
        glDetachShader(m_id, gShader);
    }
}

Shader::~Shader()
{
    glDeleteProgram(m_id);
}

std::shared_ptr<Shader> Shader::Create(const char* vertexCode, 
                                       const char* fragmentCode,
                                       const char* tesCtrlCode,
                                       const char* tesEvalCode,
                                       const char* geometryCode)
{
    return std::make_shared<Shader>(vertexCode, fragmentCode, tesCtrlCode, tesEvalCode, geometryCode);
}

std::shared_ptr<Shader> Shader::Open(const std::string& vertexPath,
                                     const std::string& fragmentPath,
                                     const std::string& tesCtrlPath,
                                     const std::string& tesEvalPath,
                                     const std::string& geometryPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::string tesCtrlCode;
    std::string tesEvalCode;
    std::string geometryCode;

    if (!ReadFile(vertexPath, vertexCode))
        return CreateEmpty();
    if (!ReadFile(fragmentPath, fragmentCode))
        return CreateEmpty();

    if (!tesCtrlPath.empty())
        ReadFile(tesCtrlPath, tesCtrlCode);
    if (!tesEvalPath.empty())
        ReadFile(tesEvalPath, tesEvalCode);
    if (!geometryPath.empty())
        ReadFile(geometryPath, geometryCode);

    return Create(vertexCode.c_str(), 
                  fragmentCode.c_str(),
                  !tesCtrlCode.empty()  ? tesCtrlCode.c_str()  : nullptr,
                  !tesEvalCode.empty()  ? tesEvalCode.c_str()  : nullptr,
                  !geometryCode.empty() ? geometryCode.c_str() : nullptr);
}

void Shader::Bind() const {
    glUseProgram(m_id);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

bool Shader::IsValid() const {
    return m_id != 0;
}

void Shader::SetInt(const std::string& name, const int& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, const float& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(location, value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}


bool Shader::CompileShader(const GLint &type, const char* shaderSource, GLuint& shaderId) const
{
    shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &shaderSource, nullptr);
    glCompileShader(shaderId);

    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        
        std::vector<char> log(length);
        glGetShaderInfoLog(shaderId, length, &length, log.data());
        fprintf(stderr, "ERROR: %s compilation failed with message :\n%s\n", Shader::ShaderTypeToText(type), log.data());

        shaderId = -1;
        return false;
    }

    return true;
}

std::shared_ptr<Shader> Shader::CreateEmpty()
{
    return std::make_shared<Shader>();
}

const char* Shader::ShaderTypeToText(const GLint &type)
{
    switch (type) {
        case GL_VERTEX_SHADER:
            return "Vertex Shader";
        case GL_FRAGMENT_SHADER:
            return "Fragment Shader";
        case GL_GEOMETRY_SHADER:
            return "Geometry Shader";
        case GL_TESS_CONTROL_SHADER:
            return "Tessellation Control Shader";
        case GL_TESS_EVALUATION_SHADER:
            return "Tessellation Evaluation Shader";
    }
    return "UnknownShaderType";
}