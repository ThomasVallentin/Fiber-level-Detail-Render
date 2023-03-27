#include "Texture3D.h"


Texture3D::Texture3D() : 
        m_width(0),
        m_height(0),
        m_depth(0),
        m_internalFormat(0) 
{
    glGenTextures(1, &m_id);
}

Texture3D::Texture3D(const GLuint& id) : 
        m_id(id),
        m_width(0),
        m_height(0),
        m_depth(0),
        m_internalFormat(0) {}

Texture3D::Texture3D(const uint32_t& width, 
                     const uint32_t& height,
                     const uint32_t& depth,
                     const GLenum& internalFormat,
                     const bool& immutable) : 
        m_width(width),
        m_height(height),
        m_depth(depth),
        m_internalFormat(internalFormat) 
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_3D, m_id);
    if (immutable)
    {
        glTexStorage3D(GL_TEXTURE_3D, 1, m_internalFormat, m_width, m_height, m_depth);
    }
    else 
    {
        glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, 
                     m_width, m_height, m_depth, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
}

Texture3D::Texture3D(const uint32_t& width, 
                     const uint32_t& height,
                     const uint32_t& depth,
                     const GLenum& internalFormat,
                     const GLenum& dataFormat,
                     const GLenum& dataType,
                     const void* data,
                     const bool& immutable) : 
        m_width(width),
        m_height(height),
        m_depth(depth),
        m_internalFormat(internalFormat) 
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_3D, m_id);
    if (immutable)
    {
        glTexStorage3D(GL_TEXTURE_3D, 1, m_internalFormat, m_width, m_height, m_depth);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                        m_width, m_height, m_depth,
                        dataFormat, dataType, data);
    }
    else 
    {
    glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, 
                 m_width, m_height, m_depth, 0, 
                 dataFormat, dataType, data);
    }
}

Texture3D::~Texture3D() {
    glDeleteTextures(1, &m_id);
    m_id = 0;
}

void Texture3D::Bind() const {
    glBindTexture(GL_TEXTURE_3D, m_id);
}

void Texture3D::Unbind() const {
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture3D::Resize(const uint32_t& width, const uint32_t& height, const uint32_t& depth) {
    // /!\ Resize is only supported for mutable textures 
    glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, 
                 width, height, depth, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_width = width;
    m_height = height;
    m_depth = depth;
}

void Texture3D::Attach(const uint32_t& unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_3D, m_id);
}

void Texture3D::SetData(const void* data, 
                        const GLenum& dataFormat, 
                        const GLenum& dataType) {
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                    m_width, m_height, m_depth,
                    dataFormat, dataType, data);
}

void Texture3D::SetData(const uint32_t& width, const uint32_t& height, const uint32_t& depth, 
                        const GLenum& internalFormat, 
                        const GLenum& dataFormat, 
                        const GLenum& dataType, 
                        const void* data) {
    glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, 
                 width, height, depth, 0, 
                 dataFormat, dataType, data);
    
    m_width = width; 
    m_height = height;
    m_depth = depth;
    m_internalFormat = internalFormat;
}
void Texture3D::GetData(const GLenum& pixelFormat, const GLenum& pixelType, const uint32_t& size, void* pixels) const
{
    glGetnTexImage(GL_TEXTURE_3D, 0, pixelFormat, pixelType, size, pixels);
}

void Texture3D::GetData(const GLenum& pixelFormat, const GLenum& pixelType, void* pixels) const
{
    glGetTexImage(GL_TEXTURE_3D, 0, pixelFormat, pixelType, pixels);
}

void Texture3D::SetFilteringFlags(const GLenum& minFilter, const GLenum& magFilter) const {
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture3D::SetWrappingFlags(const GLenum& wrapS, const GLenum& wrapT, const GLenum& wrapR) const {
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapR);
}

// Todo: drive this directly through the filtering flags
void Texture3D::EnableMipmaps(const bool& enable) {
    if (enable && !m_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_3D);
    }
    
    m_mipmaps = enable;
}


void Texture3D::ClearUnit(const uint32_t& unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_3D, 0);
}
