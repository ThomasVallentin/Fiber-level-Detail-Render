#include "SelfShadows.h"

#include "Base/Resolver.h"


std::shared_ptr<Framebuffer> SelfShadows::s_densityFramebuffer;
std::shared_ptr<Framebuffer> SelfShadows::s_absorptionFramebuffer;
std::shared_ptr<Shader> SelfShadows::s_densityShader;
std::shared_ptr<Shader> SelfShadows::s_absorptionShader;


std::shared_ptr<Texture3D> SelfShadows::GenerateTexture(const SelfShadowsSettings& settings)
{
    if (!s_densityShader)
    {
        Resolver& resolver = Resolver::Get();
        s_densityShader = std::make_shared<Shader>(resolver.Resolve("src/shaders/fullScreen.vs.glsl").c_str(),
                                                   resolver.Resolve("src/shaders/selfShadowsDensity.fs.glsl").c_str());
        s_absorptionShader = std::make_shared<Shader>(resolver.Resolve("src/shaders/fullScreen.vs.glsl").c_str(),
                                                      resolver.Resolve("src/shaders/selfShadows.fs.glsl").c_str());
    }

    // Density framebuffer
    s_densityFramebuffer = std::make_shared<Framebuffer>(settings.textureSize, settings.textureSize);
    s_densityFramebuffer->Bind();
    s_densityFramebuffer->AddColorAttachment(std::make_shared<Texture2D>(settings.textureSize, 
                                                                         settings.textureSize, 
                                                                         GL_RGBA8, true));

    // Absorption framebuffer
    s_absorptionFramebuffer = std::make_shared<Framebuffer>(settings.textureSize, settings.textureSize);
    s_absorptionFramebuffer->Bind();
    auto absorptionTexture = std::make_shared<Texture2D>(settings.textureSize, 
                                                         settings.textureSize, 
                                                         GL_RGBA8, true);
    s_absorptionFramebuffer->AddColorAttachment(absorptionTexture);

    // Dummy vao to render in full screen
    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);
    glBindVertexArray(dummyVAO);

    // Sampling the ply density between [0, 2*PI/nPlyplyCount]
    float plyAngleStep = 2.0 * M_PI / (float)settings.plyCount / (float)settings.textureCount;  
    uint32_t pixelCount = settings.textureSize * settings.textureSize;
    std::vector<uint8_t> texture3DData(pixelCount * settings.textureCount);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    for (uint32_t i=0 ; i < settings.textureCount ; ++i)
    {
        // Render density
        s_densityShader->use();
        s_densityShader->setInt("uPlyCount", settings.plyCount);
        s_densityShader->setFloat("uPlyAngle", plyAngleStep * (float)i);
        s_densityShader->setFloat("uPlyRadius", settings.plyRadius);
        s_densityShader->setFloat("uDensityE", settings.densityE);
        s_densityShader->setFloat("uDensityB", settings.densityB);
        s_densityShader->setFloat("uEN", settings.eN);
        s_densityShader->setFloat("uEB", settings.eB);

        s_densityFramebuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Render absorption from density
        s_absorptionShader->use();
        s_densityFramebuffer->GetColorAttachment(0)->Bind();
        s_absorptionShader->setInt("uDensityTexture", 0);

        s_absorptionFramebuffer->Bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Accumulate 2D "slice" in a vector
        absorptionTexture->Bind();
        absorptionTexture->GetData(GL_RED, GL_UNSIGNED_BYTE, &texture3DData[pixelCount * i]);
    }
    absorptionTexture->Unbind();
    s_absorptionFramebuffer->Unbind();
    glBindVertexArray(0);

    // Generate a 3D texture from the accumulated "slices"
    // TODO: This process may be directly computed in the GPU but this seems like an heavy task
    //       We may consider doing so if this section becomes too much of a bottleneck.
    auto result = std::make_shared<Texture3D>(settings.textureSize, 
                                              settings.textureSize, 
                                              settings.textureCount, 
                                              GL_R8, GL_RED, GL_UNSIGNED_BYTE, 
                                              texture3DData.data(), 
                                              true);
    return result;
}