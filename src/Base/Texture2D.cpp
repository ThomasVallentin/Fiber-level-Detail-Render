#include "Texture2D.h"


Texture2D::Texture2D() : 
        m_width(0),
        m_height(0),
        m_internalFormat(0) 
{
    glGenTextures(1, &m_id);
}

Texture2D::Texture2D(const GLuint& id) : 
        m_id(id),
        m_width(0),
        m_height(0),
        m_internalFormat(0) {}

Texture2D::Texture2D(const uint32_t& width, 
                     const uint32_t& height,
                     const GLenum& internalFormat,
                     const bool& immutable) : 
        m_width(width),
        m_height(height),
        m_internalFormat(internalFormat) 
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    if (immutable)
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, m_internalFormat, m_width, m_height);
    }
    else 
    {
        glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, 
                     m_width, m_height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
}

Texture2D::Texture2D(const uint32_t& width, 
                     const uint32_t& height,
                     const GLenum& internalFormat,
                     const GLenum& dataFormat,
                     const GLenum& dataType,
                     const void* data,
                     const bool& immutable) : 
        m_width(width),
        m_height(height),
        m_internalFormat(internalFormat) 
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    if (immutable)
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, m_internalFormat, m_width, m_height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                        m_width, m_height, 
                        dataFormat, dataType, data);
    }
    else 
    {
        glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, 
                     m_width, m_height, 0, 
                     dataFormat, dataType, data);
    }
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_id);
    m_id = 0;
}

void Texture2D::Bind() const {
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Resize(const uint32_t& width, const uint32_t& height) {
    // /!\ Resize is only supported for mutable textures 
    glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, 
                 width, height, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_width = width;
    m_height = height;
}

void Texture2D::Attach(const uint32_t& unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::SetData(const void* data, 
                        const GLenum& dataFormat, 
                        const GLenum& dataType) {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                    m_width, m_height, 
                    dataFormat, dataType, data);
}

void Texture2D::SetData(const uint32_t& width, const uint32_t& height, 
                        const GLenum& internalFormat, 
                        const GLenum& dataFormat, 
                        const GLenum& dataType, 
                        const void* data) {
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                 width, height, 0, 
                 dataFormat, dataType, data);
    
    m_width = width; 
    m_height = height;
    m_internalFormat = internalFormat;
}
void Texture2D::GetData(const GLenum& pixelFormat, const GLenum& pixelType, const uint32_t& size, void* pixels) const
{
    glGetnTexImage(GL_TEXTURE_2D, 0, pixelFormat, pixelType, size, pixels);
}

void Texture2D::GetData(const GLenum& pixelFormat, const GLenum& pixelType, void* pixels) const
{
    glGetTexImage(GL_TEXTURE_2D, 0, pixelFormat, pixelType, pixels);
}

void Texture2D::SetFilteringFlags(const GLenum& minFilter, const GLenum& magFilter) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture2D::SetWrappingFlags(const GLenum& wrapS, const GLenum& wrapT) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

// Todo: drive this directly through the filtering flags
void Texture2D::EnableMipmaps(const bool& enable) {
    if (enable && !m_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    m_mipmaps = enable;
}


void Texture2D::ClearUnit(const uint32_t& unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2DPtr Texture2D::Create(const uint32_t& width, 
                               const uint32_t& height,
                               const GLenum& internalFormat,
                               const bool& immutable)
                                    
{
    return std::make_shared<Texture2D>(width, height, internalFormat, immutable);
}   

Texture2DPtr Texture2D::Create(const uint32_t& width,
                               const uint32_t& height,
                               const GLenum& internalFormat,
                               const GLenum& dataFormat,
                               const GLenum& dataType,
                               const void* data,
                               const bool& immutable)
{
    return std::make_shared<Texture2D>(width, 
                                       height, 
                                       internalFormat, 
                                       dataFormat,
                                       dataType,
                                       data,
                                       immutable);
}                                      