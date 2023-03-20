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

void Framebuffer::AddColorAttachment(const std::shared_ptr<Texture2D>& attachment) {
    GLenum drawBuffer = GL_COLOR_ATTACHMENT0 + m_colorAttachments.size();
    glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffer, GL_TEXTURE_2D, attachment->GetId(), 0);
    m_colorAttachments.push_back(attachment);
    m_drawBuffers.push_back(drawBuffer);

    glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
}

void Framebuffer::SetDepthAttachment(const std::shared_ptr<Texture2D>& attachment) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment->GetId(), 0);
    m_depthAttachment = attachment;
}


std::shared_ptr<Texture2D> Framebuffer::GetColorAttachment(const uint32_t index) const {
    if (index > m_colorAttachments.size()) {
        return nullptr;
    }

    return m_colorAttachments[index];
}

std::shared_ptr<Texture2D> Framebuffer::GetDepthAttachment() const {
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

void Framebuffer::Blit(const std::shared_ptr<Framebuffer>& destination) const
{
    Blit(destination->GetId());
}
