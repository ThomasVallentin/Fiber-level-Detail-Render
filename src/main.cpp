#include "Base/Camera.h"
#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"
#include "Base/Profiler.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <iostream>


// Camera controls :
// - Pan    : Alt + Middle click + Mouse move
// - Rotate : Alt + Left click   + Mouse move
// - Zoom   : Alt + Right click  + Mouse move (horizontal)
Camera camera;
glm::vec2 mousePos;


int main(int argc, char *argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto window = Window({1280, 720, "Fiber Level Detail Render"});
    auto eventCallback = [&](Event* event) {
        switch (event->GetType()) 
        {
            case EventType::WindowResized: {
                auto resizeEvent = dynamic_cast<WindowResizedEvent*>(event);
                glViewport(0, 0, 
                           resizeEvent->GetWidth(), 
                           resizeEvent->GetHeight());
                break;
            }
            case EventType::MouseMoved: {
                auto mouseEvent = dynamic_cast<MouseMovedEvent*>(event);
                mousePos.x = mouseEvent->GetPosX();
                mousePos.y = mouseEvent->GetPosY();
                break;
            }
        }

        // 
        camera.OnEvent(event);
    };
    window.SetEventCallback(eventCallback);  // Define the event callback of the application

    auto& profiler = Profiler::Init(window);
    std::vector<ProfilingScopeData> profilingScopes;
    while (!window.ShouldClose()) {
        // Making a copy of the profiler data to display it in the UI (so that we display the previous frame stats)
        profilingScopes = profiler.GetScopes();
        profiler.Clear();

        camera.Update();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            // OpenGL Rendering
            const ProfilingScope scope("Render commands");  

            // (づ｡◕‿‿◕｡)づ    Write OpenGL code here    ~(˘▾˘~)
            // shader.use();
            // shader.setMat4("uModelMatrix", glm::mat4(1.0f));
            // shader.setMat4("uViewMatrix",  camera.GetViewMatrix());
            // shader.setMat4("uProjMatrix",  camera.GetProjectionMatrix());
            // glDrawXXXX(...);
        }

        {
            // Render ImGui items
            const ProfilingScope scope("UI Rendering");  

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
                
                    ImGui::Text("Profiling:");
                    for (const auto& scope : profilingScopes)
                    {
                        float elapsedTime = scope.duration * 1000.0;
                        indentedLabel((scope.name + " :").c_str());
                        ImGui::SameLine();
                        ImGui::BeginDisabled();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::DragFloat((std::string("##") + scope.name + "TimeDrag").c_str(), &elapsedTime, 1.0f, 0.0f, 0.0f, "%.3fms");
                        ImGui::EndDisabled();
                    }
                }
            }
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        {
            // It's the SwapBuffers that actually compute the rendering, not the calls to glXXX commands
            const ProfilingScope scope("OpenGL Rendering");  
            window.Update();
        }
    }

    return 0;
}