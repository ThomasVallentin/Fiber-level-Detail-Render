#include "ShadowMap.h"

#include "Base/Resolver.h"


ShadowMap::ShadowMap(const uint32_t& resolution)
{
    Resolver& resolver = Resolver::Get();

    auto texture = Texture2D::Create(resolution, resolution, GL_DEPTH_COMPONENT24, true);
    texture->Bind();
    texture->SetWrappingFlags(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    float defaultColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    texture->SetFloatParameter(GL_TEXTURE_BORDER_COLOR, defaultColor);
    texture->Unbind();

    m_framebuffer = Framebuffer::Create(resolution, resolution);
    m_framebuffer->Bind();
    m_framebuffer->SetDepthAttachment(texture);
    m_framebuffer->UpdateBuffers();  // Explicitly set the drawbuffers to None
    m_framebuffer->Unbind();

    // Custom shader that renders the yarns as tubes instead of generating all the fibers
    m_shader = Shader(resolver.Resolve("src/shaders/noProjection3D.vs.glsl").c_str(), 
                      resolver.Resolve("src/shaders/empty.fs.glsl").c_str(),
                      resolver.Resolve("src/shaders/triangulateLine.gs.glsl").c_str(),
                      nullptr, 
                      nullptr);
}

void ShadowMap::Begin(const glm::mat4& lightViewMatrix, const glm::mat4& lightProjMatrix)
{
    // Backup viewport dimensions to restore them during End()
    glGetIntegerv( GL_VIEWPORT, m_restoreViewport );
    
    m_shader.use();
    m_shader.setMat4("uModelMatrix", glm::mat4(1.0f));
    m_shader.setMat4("uViewMatrix", lightViewMatrix);
    m_shader.setMat4("uProjMatrix", lightProjMatrix);

    m_framebuffer->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::End()
{
    m_framebuffer->Unbind();
    glViewport(m_restoreViewport[0], 
               m_restoreViewport[1], 
               m_restoreViewport[2], 
               m_restoreViewport[3]);
}
