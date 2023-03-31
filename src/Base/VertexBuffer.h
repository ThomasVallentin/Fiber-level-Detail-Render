#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <memory>


class VertexBuffer;
class IndexBuffer;


using VertexBufferPtr = std::shared_ptr<VertexBuffer>;
using IndexBufferPtr = std::shared_ptr<IndexBuffer>;


static GLuint SizeOfGLType(const GLenum& type) {
    switch (type) {
            case GL_BOOL:            return 1;
            case GL_BYTE:            return 1;
            case GL_UNSIGNED_BYTE:   return 1;
            case GL_SHORT:           return 2;
            case GL_UNSIGNED_SHORT:  return 2;
            case GL_INT:             return 4;
            case GL_UNSIGNED_INT:    return 4;
            case GL_FLOAT:           return 4;
            case GL_HALF_FLOAT:      return 2;
            case GL_DOUBLE:          return 8;
            default:                 return 0;
    }
}


// == VertexBufferLayout ==

struct VertexBufferAttribute 
{
    VertexBufferAttribute(const std::string& name,
                          const GLint& dimension,
                          const GLenum& type,
                          const GLboolean& normalized) :
            name(name),
            dimension(dimension),
            type(type),
            offset(0),
            normalized(normalized) {}

    std::string name;
    GLuint dimension;
    GLuint type;  
    GLuint offset;
    GLboolean normalized;
};


class VertexBufferLayout
{
public:
    typedef std::vector<VertexBufferAttribute> VertexBufferAttrVector;
    
    VertexBufferLayout() = default;
    VertexBufferLayout(const std::initializer_list<VertexBufferAttribute>& attrs) :
            m_attributes(attrs) 
    {
        m_stride = 0;
        for (auto& attribute : m_attributes) {
            attribute.offset = m_stride;
            m_stride += SizeOfGLType(attribute.type) * attribute.dimension;
        }
    }

    inline const VertexBufferAttrVector &GetAttributes() const { return m_attributes; }
    inline GLuint GetStride() const { return m_stride + m_offset; };
    inline void SetOffset(const GLuint& offset) { m_offset = offset; };

private:
    VertexBufferAttrVector m_attributes;
    GLuint m_stride = 0;
    GLuint m_offset = 0;
};


// == VertexBuffer ==

class VertexBuffer
{
public:
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    VertexBufferLayout GetLayout() const;
    void SetLayout(const VertexBufferLayout& layout);
    void SetData(const void* data, const GLuint& size) const;

    static VertexBufferPtr Create();
    static VertexBufferPtr Create(const void* data, const GLuint& size);

private:
    VertexBuffer();

    GLuint m_id = 0;
    VertexBufferLayout m_layout;
};


// == IndexBuffer ==

class IndexBuffer
{
public:
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    void SetData(const GLuint* indices, const GLuint& count);
    inline GLuint GetCount() const { return m_count; }

    static IndexBufferPtr Create();
    static IndexBufferPtr Create(const GLuint* indices, const uint32_t& count);

private:
    IndexBuffer();

    GLuint m_id = 0;
    uint32_t m_count;

};


#endif  // VERTEXBUFFER_H