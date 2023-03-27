#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Texture2D.h"
#include <memory>

class Framebuffer;
using FramebufferPtr = std::shared_ptr<Framebuffer>;

class Framebuffer
{
public:
    Framebuffer(const uint32_t& width, const uint32_t& height);
    ~Framebuffer();

    inline GLuint GetId() const { return m_id; }

    void Bind();
    void Unbind();

    inline uint32_t GetWidth() const { return m_width; } 
    inline uint32_t GetHeight() const { return m_height; } 
    void Resize(const uint32_t& width, const uint32_t& height);

    void AddColorAttachment(const Texture2DPtr& attachment);
    void SetDepthAttachment(const Texture2DPtr& attachment, const bool& depthStencil=false);
    Texture2DPtr GetColorAttachment(const uint32_t index) const;
    Texture2DPtr GetDepthAttachment() const;
    void UpdateBuffers();
    
    void Blit(const GLuint& destId) const;
    void Blit(const FramebufferPtr& destination) const;

    static FramebufferPtr Create(const uint32_t& width, const uint32_t& height);

private:
    GLuint m_id;

    uint32_t m_width, m_height;

    std::vector<Texture2DPtr> m_colorAttachments;
    std::vector<GLenum> m_drawBuffers;
    Texture2DPtr m_depthAttachment;
};


#endif
