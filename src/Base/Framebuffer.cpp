#include "Framebuffer.h"


Framebuffer::Framebuffer(const uint32_t& width, const uint32_t& height) :
        m_width(width),
        m_height(height)
{
    glGenFramebuffers(1, &m_id);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_id);
    m_id = 0;
}

void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(const uint32_t& width, const uint32_t& height) {
    if (m_width == width && m_height == height) {
        return;
    }

    for (auto& attachment : m_colorAttachments) {
        attachment->Resize(width, height);
    }

    if (m_depthAttachment)
    {
        m_depthAttachment->Resize(width, height);
    }

    m_width = width;
    m_height = height;
}

void Framebuffer::AddColorAttachment(const Texture2DPtr& attachment) {
    GLenum drawBuffer = GL_COLOR_ATTACHMENT0 + m_colorAttachments.size();
    glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffer, GL_TEXTURE_2D, attachment->GetId(), 0);
    m_colorAttachments.push_back(attachment);
    m_drawBuffers.push_back(drawBuffer);

    glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
}

void Framebuffer::SetDepthAttachment(const Texture2DPtr& attachment, const bool& depthStencil) {
    if (depthStencil)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment->GetId(), 0);
    }
    else 
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachment->GetId(), 0);
    }
    m_depthAttachment = attachment;
}

void Framebuffer::UpdateBuffers()
{
    glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
}

Texture2DPtr Framebuffer::GetColorAttachment(const uint32_t index) const {
    if (index > m_colorAttachments.size()) {
        return nullptr;
    }

    return m_colorAttachments[index];
}

Texture2DPtr Framebuffer::GetDepthAttachment() const {
    return m_depthAttachment;
}

void Framebuffer::Blit(const GLuint& destFrameBufferId) const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBufferId);
    glBlitFramebuffer(0, 0, m_width, m_height, 
                      0, 0, m_width, m_height, 
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Restore the current framebuffer as the drawing buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id);
}

void Framebuffer::Blit(const FramebufferPtr& destination) const
{
    Blit(destination->GetId());
}

FramebufferPtr Framebuffer::Create(const uint32_t& width, const uint32_t& height)
{
    return std::make_shared<Framebuffer>(width, height);
}
