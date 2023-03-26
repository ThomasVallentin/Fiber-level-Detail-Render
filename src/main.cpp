#include "SelfShadows.h"
#include "ShadowMap.h"

#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"
#include "Base/Shader.h"
#include "Base/Framebuffer.h"
#include "Base/VertexArray.h"
#include "Base/Camera.h"
#include "Base/bccReader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>

#include <iostream>


// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera - give pretty starting point
Camera camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float prevTime = 0.0f;

// Rendering parameters
bool useAmbientOcclusion = true;
bool useShadowMapping = true;

float shadowMapThickness = 0.15f;

int main(int argc, char *argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto window = Window({1280, 720, "Fiber-Level Detail Render"});


    auto eventCallback = [&](Event* event) {
        switch (event->GetType()) 
        {
            case EventType::WindowResized: {
                auto resizeEvent = dynamic_cast<WindowResizedEvent*>(event);
                glViewport(0, 0, resizeEvent->GetWidth(), resizeEvent->GetHeight());
                camera.SetViewportSize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
                break;
            }
        }

        camera.OnEvent(event);
    };
    window.SetEventCallback(eventCallback);

    // Read curves from the BCC file and send them to OpenGL
    std::vector<std::vector<glm::vec3>> closedFibersCP;
    std::vector<std::vector<glm::vec3>> openFibersCP;
    readBCC(resolver.Resolve("resources/fiber.bcc"), closedFibersCP, openFibersCP);

    // Send the data to OpenGL
    // openFibersCP[0] = {{0, 0, 0}, {0.5, 0.5, 0.25}, {1, 0, 0.5}, {1.5, 0.5, 0.25}, 
    //                     {2, 0, 0}, {2.5, 0.5, 0.25}, {3, 0, 0.5}, {3.5, 0.5, 0.25}};

    uint32_t fibersVertexCount = openFibersCP[0].size();
    auto fibersVertexBuffer = VertexBuffer::Create(openFibersCP[0].data(), 
                                                   fibersVertexCount * sizeof(glm::vec3));
    fibersVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});

    std::vector<uint32_t> indices;
    for (size_t i = 0 ; i < fibersVertexCount - 3 ; i++)
    {
        indices.push_back(i);
        indices.push_back(i+1);
        indices.push_back(i+2);
        indices.push_back(i+3);
    }
    auto fibersIndexBuffer = IndexBuffer::Create(indices.data(), 
                                                 indices.size());

    auto fibersVertexArray = VertexArray::Create();
    fibersVertexArray->Bind();
    fibersVertexArray->AddVertexBuffer(fibersVertexBuffer);
    fibersVertexArray->SetIndexBuffer(fibersIndexBuffer);
    fibersVertexArray->Unbind();
    
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    Shader fiberShader(resolver.Resolve("src/shaders/fibers.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/fibers.fs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.gs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tsc.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tse.glsl").c_str());

    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);
    glBindVertexArray(dummyVAO);

    // Shadow mapping
    DirectionalLight directional(glm::normalize(glm::vec3(0.5f, -0.5f, -0.5f)), {0.8f, 0.8f, 0.8f});
    ShadowMap shadowMap(4096);
    
    Shader blitChannelShader(resolver.Resolve("src/shaders/utility/fullScreen.vs.glsl").c_str(), 
                             resolver.Resolve("src/shaders/utility/blitTextureChannel.fs.glsl").c_str());

    // Simple plane to visualize the casted shadows
    std::vector<Vertex> planeVertices = {{{-10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {0.0, 0.0}},
                                         {{-10.0, 0.0,  10.0}, {0.0, 1.0, 0.0}, {0.0, 1.0}},
                                         {{ 10.0, 0.0,  10.0}, {0.0, 1.0, 0.0}, {1.0, 1.0}},
                                         {{ 10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {1.0, 0.0}}};
    std::vector<uint32_t> planeIndices = {0, 1, 2, 2, 3, 0};
    auto planeVertexBuffer = VertexBuffer::Create(planeVertices.data(), 
                                                  planeVertices.size() * sizeof(Vertex));
    planeVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false},
                                  {"Normal",    3, GL_FLOAT, false},
                                  {"TexCoord",  2, GL_FLOAT, false}});
    auto planeIndexBuffer = IndexBuffer::Create(planeIndices.data(), planeIndices.size()); 
    auto planeVertexArray = VertexArray::Create();
    planeVertexArray->Bind();
    planeVertexArray->AddVertexBuffer(planeVertexBuffer);
    planeVertexArray->SetIndexBuffer(planeIndexBuffer);
    planeVertexArray->Unbind();

    Shader planeShader(resolver.Resolve("src/shaders/default3D.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/lambert.fs.glsl").c_str());

    // Self Shadows
    // SelfShadowsSettings selfShadowsSettings{512, 16};
    // std::shared_ptr<Texture3D> selfShadowsTexture = SelfShadows::GenerateTexture(selfShadowsSettings);    

    // Shader slice3DShader(resolver.Resolve("src/shaders/utility/fullScreen.vs.glsl").c_str(),
    //                      resolver.Resolve("src/shaders/utility/3DTextureSlice.fs.glsl").c_str());

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    while (!window.ShouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;
        
        camera.Update();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projMatrix = camera.GetProjectionMatrix();
        glm::mat4 viewMatrix = camera.GetViewMatrix();
        glm::mat4 viewInverseMatrix = glm::inverse(camera.GetViewMatrix());
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Render the shadow map
        // directional.SetDirection(camera.GetForwardDirection());
        directional.SetDirection(glm::vec3(glm::rotate(glm::mat4(1.0f), 0.1f, {0.0f, 1.0f, 0.0f}) * glm::vec4(directional.GetDirection(), 1.0f)));
        if (useShadowMapping)
        {    
            shadowMap.Begin(directional.GetViewMatrix(), directional.GetProjectionMatrix(), shadowMapThickness);
            {
                // Render all the objects that cast shadows here
                fibersVertexArray->Bind();
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glDrawElements(GL_PATCHES, fibersIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
                glDisable(GL_CULL_FACE);
                fibersVertexArray->Unbind();
            }    
            shadowMap.End();
        }
        else
        {
            shadowMap.Clear();
        }

        fiberShader.use();
        fiberShader.setMat4("uProjMatrix", projMatrix);
        fiberShader.setMat4("uViewMatrix", viewMatrix);
        fiberShader.setMat4("uModelMatrix", modelMatrix);
    
        fibersVertexArray->Bind();

        fiberShader.setFloat("R_ply", 0.1f);
        fiberShader.setFloat("Rmin", 0.1f);
        fiberShader.setFloat("Rmax", 0.2f);
        fiberShader.setFloat("theta", 1.0f);
        fiberShader.setFloat("s", 2.0f);  // length of rotation
        fiberShader.setFloat("eN", 1.0f); // ellipse scaling factor along Normal
        fiberShader.setFloat("eB", 1.0f); // ellipse scaling factor along Bitangent

        fiberShader.setFloat("R[0]", 0.20f); // distance from fiber i to ply center
        fiberShader.setFloat("R[1]", 0.25f); // distance from fiber i to ply center
        fiberShader.setFloat("R[2]", 0.30f); // distance from fiber i to ply center
        fiberShader.setFloat("R[3]", 0.35f); // distance from fiber i to ply center

        // Fragment related uniforms
        fiberShader.setBool("uUseAmbientOcclusion", useAmbientOcclusion); // distance from fiber i to ply center
        if (useShadowMapping)
        {
            fiberShader.setMat4("uViewToLightMatrix", directional.GetProjectionMatrix() * directional.GetViewMatrix() * viewInverseMatrix);
            shadowMap.GetTexture()->Attach(0);
            fiberShader.setInt("uShadowMap", 0);
        }
        else 
        {
            fiberShader.setMat4("uViewToLightMatrix", glm::mat4(0.0));
            Texture2D::ClearUnit(0);
            fiberShader.setInt("uShadowMap", 0);
        }

        glDrawElements(GL_PATCHES, fibersIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
        
        // Render slices of selfShadow to screen
        // slice3DShader.use();
        // selfShadowsTexture->Attach(0);
        // slice3DShader.setInt("uInputTexture", 0);
        // slice3DShader.setFloat("uDepth", std::sin(currentTime) * 0.5 + 0.5);

        // glBindVertexArray(dummyVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0);

        // // Render plane
        // planeShader.use();
        // planeShader.setMat4("uModelMatrix", glm::mat4(1.0f));
        // planeShader.setMat4("uViewMatrix", camera.GetViewMatrix());
        // planeShader.setMat4("uProjMatrix", camera.GetProjectionMatrix());
        // planeShader.setMat4("uLightSpaceMatrix", directional.GetProjectionMatrix() * directional.GetViewMatrix());
        // shadowMap.GetTexture()->Attach(0);
        // planeShader.setInt("uShadowMap", 0);

        // planeVertexArray->Bind();
        // glDrawElements(GL_TRIANGLES, planeIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
        // planeVertexArray->Unbind();

        // // Blit shadow map to the screen
        // blitChannelShader.use();
        // shadowMap.GetTexture()->Attach(0);
        // blitChannelShader.setInt("uInput", 0);
        // glBindVertexArray(dummyVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto& io = ImGui::GetIO();
        ImGui::Begin("Control panel", nullptr);
        {
            if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
            {
                indentedLabel("FPS:");
                ImGui::SameLine();
                ImGui::Text("%.1f (%.3fms)", io.Framerate, 1000.0f / io.Framerate);
            }
            if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
            {
                indentedLabel("Ambient occlusion:");
                ImGui::SameLine();
                ImGui::Checkbox("##UseAmbientOcclusion", &useAmbientOcclusion);

                indentedLabel("Shadow Mapping:");
                ImGui::SameLine();
                ImGui::Checkbox("##UseShadowMapping", &useShadowMapping);

                indentedLabel("Shadow Map Thickess:");
                ImGui::SameLine();
                ImGui::DragFloat("##ShadowMapThicknessSlider", &shadowMapThickness, 0.001f, 0.0f, 1.0);
            }
        }

        ImGui::End();

        // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        window.Update();
    }

    return 0;
}
