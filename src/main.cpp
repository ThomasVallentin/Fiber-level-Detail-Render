#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer/Shader.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "ERROR: Glfw error %d: %s\n", error, description);
}


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Setup Glfw & OpenGL versions
    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);          

    // Create window & graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    // // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    printf("Successfully loaded OpenGL !\n"
             "            Vendor: %s\n"
             "            Renderer: %s\n"
             "            Version: %s\n", 
              glGetString(GL_VENDOR), 
              glGetString(GL_RENDERER), 
              glGetString(GL_VERSION));


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

	glEnable (GL_LINE_SMOOTH);

    bool show_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    float vertices[] = {
        -0.75f, 0.0f, 0.0f,
        -0.25f, 0.2f, 0.0f,
         0.25f, 0.0f, 0.0f,
         0.75f, 0.1f, 0.0f,
    };  
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Setting patch length to 4
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    auto shader = Shader::Open("/home/tvallentin/Projects/OpenGLTests/resources/shaders/default.vert", 
                               "/home/tvallentin/Projects/OpenGLTests/resources/shaders/default.frag",
                               "/home/tvallentin/Projects/OpenGLTests/resources/shaders/fibers.tesc",
                               "/home/tvallentin/Projects/OpenGLTests/resources/shaders/fibers.tese",
                            //    "",
                            //    "",
                               "");
    shader->Bind();
    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // // 3. Show another simple window.
        if (show_window)
        {
            ImGui::Begin("Another Window", &show_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_window = false;
            ImGui::End();
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_PATCHES, 0, 4);
        // glDrawArrays(GL_LINE_STRIP, 0, 4);

        // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
