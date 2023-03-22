#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"
#include "Base/Shader.h"
#include "Base/Camera.h"
#include "Base/bccReader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main(int argc, char *argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: REAL TIME FIBER RENDER", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    /// OPENGL program start here 

    std::vector<std::vector<glm::vec3>> closedFibersCP;
    std::vector<std::vector<glm::vec3>> openFibersCP;

    Shader shader("../src/shaders/shader.vs.glsl", "../src/shaders/shader.fs.glsl",nullptr,"../src/shaders/shader.tsc.glsl","../src/shaders/shader.tse.glsl");

    readBCC("../resources/fiber.bcc",closedFibersCP,openFibersCP);

    float vertices[] = {
        -0.75f, -0.0f, 0.0f, // pos1
         -0.25f, 0.25f, 0.0f, // pos2 
         0.25f, -0.25f, 0.0f, // pos3 
         0.75f, -0.0f, 0.0f, // pos4
         1.0f, 0.2f, 0.0f // pos5
    }; 

    // see bezier curve definition @ https://www.gatevidyalay.com/bezier-curve-in-computer-graphics-examples/
 
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::scale(model,glm::vec3(4.0f));
        shader.setMat4("model", model);
    
        shader.use();
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


        glDrawArrays(GL_PATCHES,0,4);
        glDrawArrays(GL_PATCHES,1,4);

        // // Start the Dear ImGui frame
        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // auto& io = ImGui::GetIO();
        // ImGui::Begin("Control panel", nullptr);
        // {
        //     if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
        //     {
        //         indentedLabel("FPS:");
        //         ImGui::SameLine();
        //         ImGui::Text("%.1f (%.3fms)", io.Framerate, 1000.0f / io.Framerate);
        //     }
        // }

        // ImGui::End();

        // // Render ImGui items
        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
