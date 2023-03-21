#ifndef SELFSHADOWS_H
#define SELFSHADOWS_H
/*
      Self-Shadows

Step 1 : Precompute n slices of the yarn -> store in 3D texture ordered by the angle of incidence of the light (== rotation of the yarn if the light stays still) :
    - [0°:(2D Texture), 20°:(2D Texture), 40°:(2D Texture), ...,  2*PI/plyCount:(2D Texture)]
Step 2 : Inside the fragment shader, get the light direction relative to the yarn orientation
Step 3 : Get the point to shade expressed in the space expressed in Step 2.
Step 3 : Sample the 3D texture based on the light direction / orientation (interpolate between the most suited slices) and the point we want to shade.
         Sample

*/

#include "Base/Framebuffer.h"
#include "Base/Texture2D.h"
#include "Base/Texture3D.h"
#include "Base/Shader.h"

struct SelfShadowsSettings
{
    float textureSize = 512;
    uint32_t textureCount = 16;

    uint32_t plyCount = 3;
    float plyRadius = 1.75;
    float densityE = 0.25;
    float densityB = 0.75;
    float eN = 1.3;
    float eB = 1.0;
};


class SelfShadows
{
public:
    static std::shared_ptr<Texture3D> GenerateTexture(const SelfShadowsSettings& settings);

private:
    SelfShadows() = delete;
    SelfShadows(const SelfShadows&) = delete;

    static FramebufferPtr s_densityFramebuffer;
    static FramebufferPtr s_absorptionFramebuffer;
    static ShaderPtr s_densityShader;
    static ShaderPtr s_absorptionShader;
};


#endif // SELFSHADOWS_H