#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <iostream>


int main(int argc, char *argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto window = Window({1280, 720, "Fiber Level Detail Render"});

    glm::vec2 mousePos;

    // Define the event callback of the application
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
    };
    window.SetEventCallback(eventCallback);

    while (!window.ShouldClose()) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // (づ｡◕‿‿◕｡)づ    Write fancy code here    ~(˘▾˘~)


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

                indentedLabel("Mouse pos:");
                ImGui::SameLine();
                ImGui::Text("%.1f, %.1f", mousePos.x, mousePos.y);
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