#ifndef SHADOWMAP_H
#define SHADOWMAP_H


#include "DirectionalLight.h"

#include "Base/Framebuffer.h"
#include "Base/Shader.h"

#include <glm/glm.hpp>


class ShadowMap
{
public:
    ShadowMap(const uint32_t& resolution=1024);
    ~ShadowMap() = default;

    inline Texture2DPtr GetTexture() const { return m_framebuffer->GetDepthAttachment(); };
    inline FramebufferPtr GetFramebuffer() const { return m_framebuffer; };

    void Begin(const glm::mat4& lightViewMatrix, const glm::mat4& lightProjMatrix);
    void End();

private:
    FramebufferPtr m_framebuffer;
    Shader m_shader;
};


#endif