#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>

class Shader {
public:
    void Bind() const;
    void Unbind() const;
    bool IsValid() const;
    uint32_t Id() const;

    void SetBool(const std::string& name, const bool& value) const;
    void SetInt(const std::string& name, const int& value) const;
    void SetFloat(const std::string& name, const float& value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat3(const std::string& name, const glm::mat3& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

    static std::shared_ptr<Shader> Create(const char* vertexCode, 
                                          const char* fragmentCode,
                                          const char* tesCtrlCode=nullptr,
                                          const char* tesEvalCode=nullptr,
                                          const char* geometryCode=nullptr);
    static std::shared_ptr<Shader> Open(const std::string& vertexPath,
                                        const std::string& fragmentPath,
                                        const std::string& tesCtrlPath="",
                                        const std::string& tesEvalPath="",
                                        const std::string& geometryPath=""
                                        );
    
    Shader();
    Shader(const char* vertexCode, 
           const char* fragmentCode,
           const char* tesCtrlCode,
           const char* tesEvalCode,
           const char* geometryCode);
    ~Shader();

    static const char* ShaderTypeToText(const GLint &type);

private:
    bool CompileShader(const GLint &type, const char* shaderSource, GLuint& outId) const;
    
    static std::shared_ptr<Shader> CreateEmpty();

    uint32_t m_id;
};

#endif
