#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "VertexBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>


class VertexArray;
using VertexArrayPtr = std::shared_ptr<VertexArray>;


// == VertexArray ==

class VertexArray
{
    typedef std::vector<VertexBufferPtr> VertexBufferVector;

public:
    virtual ~VertexArray();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    void AddVertexBuffer(std::shared_ptr<VertexBuffer> &vertexBuffer);
    inline const VertexBufferVector& GetVertexBuffers() const { return m_vertexBuffers; }

    void SetIndexBuffer(std::shared_ptr<IndexBuffer> &indexBuffer);
    inline const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer; }

    static VertexArrayPtr Create();

private:
    VertexArray();

    VertexBufferVector m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;

    GLuint m_attributeCount = 0;
    GLuint m_id = 0;
};


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};


#endif  // VERTEXARRAY_H
