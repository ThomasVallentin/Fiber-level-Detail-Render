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
    auto fibersVertexBuffer = VertexBuffer::Create(openFibersCP[0].data(), 
                                                   openFibersCP[0].size() * sizeof(glm::vec3));
    fibersVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});
    auto fibersVertexArray = VertexArray::Create();
    fibersVertexArray->Bind();
    fibersVertexArray->AddVertexBuffer(fibersVertexBuffer);
    fibersVertexArray->Unbind();
    
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    Shader fiberShader(resolver.Resolve("src/shaders/noProjection3D.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/solidColor.fs.glsl").c_str(),
                       resolver.Resolve("src/shaders/triangulateLine.gs.glsl").c_str(),
                       nullptr, 
                       nullptr);

    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);
    glBindVertexArray(dummyVAO);

    // Shadow mapping
    DirectionalLight directional;
    ShadowMap shadowMap;

    Shader blitChannelShader(resolver.Resolve("src/shaders/fullScreen.vs.glsl").c_str(), 
                             resolver.Resolve("src/shaders/blitTextureChannel.fs.glsl").c_str());

    // Self Shadows
    // SelfShadowsSettings selfShadowsSettings{512, 16};
    // std::shared_ptr<Texture3D> selfShadowsTexture = SelfShadows::GenerateTexture(selfShadowsSettings);    

    // Shader slice3DShader(resolver.Resolve("src/shaders/fullScreen.vs.glsl").c_str(),
    //                      resolver.Resolve("src/shaders/3DTextureSlice.fs.glsl").c_str());

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    while (!window.ShouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;
        
        camera.Update();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // basicShader.use();
        // basicShader.setMat4("uViewProjMatrix", camera.GetViewProjectionMatrix());

        // glDrawArrays(GL_LINE_STRIP, 0, openFibersCP[0].size());
        
        // Render slices of selfShadow to screen
        // slice3DShader.use();
        // selfShadowsTexture->Attach(0);
        // slice3DShader.setInt("uInputTexture", 0);
        // slice3DShader.setFloat("uDepth", std::sin(currentTime) * 0.5 + 0.5);

        // glBindVertexArray(dummyVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0);

        // fiberShader.use();
        // fiberShader.setMat4("uModelMatrix", glm::mat4(1.0f));
        // fiberShader.setMat4("uViewMatrix", directional.GetViewMatrix());
        // fiberShader.setMat4("uProjMatrix", directional.GetProjectionMatrix());
    
        directional.SetDirection(camera.GetForwardDirection());

        shadowMap.Begin(directional.GetViewMatrix(), directional.GetProjectionMatrix());
        {
            fibersVertexArray->Bind();
            glDrawArrays(GL_LINE_STRIP, 0, openFibersCP[0].size());
            fibersVertexArray->Unbind();
        }
        shadowMap.End();
        
        blitChannelShader.use();
        shadowMap.GetTexture()->Attach(0);
        blitChannelShader.setInt("uInput", 0);

        glBindVertexArray(dummyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

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
        }

        ImGui::End();

        // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        window.Update();
    }

    return 0;
}
