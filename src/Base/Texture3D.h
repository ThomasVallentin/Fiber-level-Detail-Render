#ifndef TEXTURE3D_H
#define TEXTURE3D_H

#include <glad/glad.h>

#include <vector>
#include <memory>
#include <stdint.h>


class Texture3D
{
public:
    Texture3D(); 
    Texture3D(const uint32_t& width, 
              const uint32_t& height,
              const uint32_t& depth,
              const GLenum& internalFormat,
              const bool& immutable=false);
    Texture3D(const uint32_t& width, 
              const uint32_t& height,
              const uint32_t& depth,
              const GLenum& internalFormat,
              const GLenum& dataFormat,
              const GLenum& dataType,
              const void* data,
              const bool& immutable=false);
    Texture3D(const GLuint& id);
    ~Texture3D();

    inline GLuint GetId() { return m_id; }
    
    void Bind() const;
    void Attach(const uint32_t& unit) const;
    void Unbind() const;

    inline uint32_t GetWidth() const { return m_width; } 
    inline uint32_t GetHeight() const { return m_height; } 
    inline uint32_t GetDepth() const { return m_depth; } 
    void Resize(const uint32_t& width, const uint32_t& height, const uint32_t& depth);

    void SetData(const void* data, const GLenum& dataFormat, const GLenum& dataType);
    void SetData(const uint32_t& width, const uint32_t& height, const uint32_t& depth, 
                 const GLenum& internalFormat, 
                 const GLenum& dataFormat, 
                 const GLenum& dataType, 
                 const void* data);
    void GetData(const GLenum& pixelFormat, const GLenum& pixelType, const uint32_t& size, void* pixels) const;
    void GetData(const GLenum& pixelFormat, const GLenum& pixelType, void* pixels) const;

    void SetFilteringFlags(const GLenum& minFilter, const GLenum& magFilter) const;
    void SetWrappingFlags(const GLenum& wrapS, 
                          const GLenum& wrapT, 
                          const GLenum& wrapR) const;
    void EnableMipmaps(const bool& enable);

    static void ClearUnit(const uint32_t& unit);

private:
    GLuint m_id;

    uint32_t m_width, m_height, m_depth;
    GLenum m_internalFormat;
    bool m_mipmaps = false;
};


#endif