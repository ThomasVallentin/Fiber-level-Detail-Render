#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Texture2D.h"


class Framebuffer
{
public:
    Framebuffer(const uint32_t& width, const uint32_t& height);
    ~Framebuffer();

    inline GLuint getId() const { return m_id; }

    void Bind();
    void Unbind();

    inline uint32_t GetWidth() const { return m_width; } 
    inline uint32_t GetHeight() const { return m_height; } 
    void Resize(const uint32_t& width, const uint32_t& height);

    void AddColorAttachment(const std::shared_ptr<Texture2D>& attachment);
    void SetDepthAttachment(const std::shared_ptr<Texture2D>& attachment);
    std::shared_ptr<Texture2D> GetColorAttachment(const uint32_t index) const;
    std::shared_ptr<Texture2D> GetDepthAttachment() const;

    void Blit(const GLuint& destId) const;
    void Blit(const std::shared_ptr<Framebuffer>& destination) const;

private:
    GLuint m_id;

    uint32_t m_width, m_height;

    std::vector<std::shared_ptr<Texture2D>> m_colorAttachments;
    std::vector<GLenum> m_drawBuffers;
    std::shared_ptr<Texture2D> m_depthAttachment;
};


#endif
