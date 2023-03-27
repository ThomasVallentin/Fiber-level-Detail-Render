#include "VertexBuffer.h"


// == VertexBuffer ==

VertexBuffer::VertexBuffer()
{
    glGenBuffers(1, &m_id);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool VertexBuffer::IsValid() const
{
    return m_id;
}

VertexBufferLayout VertexBuffer::GetLayout() const
{
    return m_layout;
}

void VertexBuffer::SetLayout(const VertexBufferLayout& layout)
{
    m_layout = layout;
}

void VertexBuffer::SetData(const void* data, const GLuint& size) const
{
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBufferPtr VertexBuffer::Create()
{
    VertexBuffer* buffer = new VertexBuffer();
    return VertexBufferPtr(buffer);
}

VertexBufferPtr VertexBuffer::Create(void* data, const GLuint& size)
{
    VertexBuffer* buffer = new VertexBuffer();
    buffer->Bind();
    buffer->SetData(data, size);
    buffer->Unbind();
    
    return VertexBufferPtr(buffer);
}


// == IndexBuffer ==

IndexBuffer::IndexBuffer()
{
    glGenBuffers(1, &m_id);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool IndexBuffer::IsValid() const
{
    return m_id;
}

void IndexBuffer::SetData(const GLuint* indices, const uint32_t& count) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    m_count = count;
}

IndexBufferPtr IndexBuffer::Create()
{
    IndexBuffer* buffer = new IndexBuffer();
    return IndexBufferPtr(buffer);
}

IndexBufferPtr IndexBuffer::Create(const GLuint* indices, const uint32_t& count)
{
    IndexBuffer* buffer = new IndexBuffer();
    buffer->Bind();
    buffer->SetData(indices, count);
    buffer->Unbind();
    
    return IndexBufferPtr(buffer);
}
