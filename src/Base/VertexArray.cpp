#include "VertexArray.h"


VertexArray::VertexArray() : m_id(0)
{
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
    m_id = 0;
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

bool VertexArray::IsValid() const
{
    return m_id;
}

void VertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> &vertexBuffer)
{
    glBindVertexArray(m_id);
    vertexBuffer->Bind();

    const auto& layout = vertexBuffer->GetLayout();
    for (const auto& attribute : layout.GetAttributes()) {
        glVertexAttribPointer(m_attributeCount, 
                              attribute.dimension, 
                              attribute.type, 
                              attribute.normalized, 
                              layout.GetStride(), 
                              (const void*)attribute.offset);
        glEnableVertexAttribArray(m_attributeCount);

        m_attributeCount++;
    }

    m_vertexBuffers.push_back(vertexBuffer);

    glBindVertexArray(0);
}

void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> &indexBuffer)
{
    glBindVertexArray(m_id);
    indexBuffer->Bind();

    m_indexBuffer = indexBuffer;
    
    glBindVertexArray(0);
}

VertexArrayPtr VertexArray::Create()
{
    VertexArray* vtxArray = new VertexArray();
    return VertexArrayPtr(vtxArray);
}
