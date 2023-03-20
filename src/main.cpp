#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"
#include "Base/Shader.h"
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
float lastFrame = 0.0f;

int main(int argc, char *argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto win = Window({1280, 720, "Fiber-Level Detail Render"});
    auto window = win.GetInternalWindow();

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
    win.SetEventCallback(eventCallback);

    /// OPENGL program start here 

    std::vector<std::vector<glm::vec3>> closedFibersCP;
    std::vector<std::vector<glm::vec3>> openFibersCP;
    readBCC(resolver.Resolve("resources/fiber.bcc"), closedFibersCP, openFibersCP);

    Shader shader(resolver.Resolve("src/shaders/shader.vs.glsl").c_str(), 
                  resolver.Resolve("src/shaders/shader.fs.glsl").c_str(),nullptr,
                  resolver.Resolve("src/shaders/shader.tsc.glsl").c_str(),
                  resolver.Resolve("src/shaders/shader.tse.glsl").c_str());

    float vertices[] = {
        -0.75f, -0.0f, 0.0f, // left  
         -0.25f, 0.25f, 0.0f, // top 
         0.25f, -0.25f, 0.0f, // bottom 
         0.75f, -0.0f, 0.0f, // right 
    }; 

    // see bezier curve definition @ https://www.gatevidyalay.com/bezier-curve-in-computer-graphics-examples/
 
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, openFibersCP[0].size() * sizeof(glm::vec3), openFibersCP[0].data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    while (!win.ShouldClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        camera.Update();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        
        glm::mat4 projection = camera.GetProjectionMatrix();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);
    
        glBindVertexArray(VAO);     

        shader.setFloat("R_ply", 0.1f);
        shader.setFloat("Rmin", 0.1f);
        shader.setFloat("Rmax", 0.2f);
        shader.setFloat("theta", 1.0f);
        shader.setFloat("s", 2.0f);  // length of rotation
        shader.setFloat("eN", 1.0f); // ellipse scaling factor along Normal
        shader.setFloat("eB", 1.0f); // ellipse scaling factor along Bitangent

        shader.setFloat("R[0]", 0.1f); // distance from fiber i to ply center
        shader.setFloat("R[1]", 0.15f); // distance from fiber i to ply center
        shader.setFloat("R[2]", 0.05f); // distance from fiber i to ply center
        shader.setFloat("R[3]", 0.2f); // distance from fiber i to ply center

        glDrawArrays(GL_PATCHES, 0, openFibersCP[0].size());

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
        
        win.Update();
    }

    return 0;
}
