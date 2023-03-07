#include "Window.h"

#include "Logging.h"
#include "Event.h"
#include "Resolver.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


Window::Window(const WindowInternalData& settings) :
        m_internalData({settings.width, 
                        settings.height,
                        settings.title.c_str()})
{
    if (!s_glfwHasBeenInitialized) {
        
        if (!glfwInit()) {
            LOG_ERROR("Something went wrong trying to initialize GLFW !");
            return;
        }
        
        glfwSetErrorCallback(Window::GLFWErrorHandler);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);          
        glfwWindowHint(GLFW_SAMPLES, 4);

        s_glfwHasBeenInitialized = true;
    }

    m_window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        LOG_ERROR("Something went wrong trying to open a GLFW window !");
        return;
    }
    glfwMakeContextCurrent(m_window);

    // Enable VSync
    glfwSwapInterval(1);
    
    // Pass internal data to the window
    m_internalData.instance = this;
    glfwSetWindowUserPointer(m_window, &m_internalData);

    // Define callbacks
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }

        WindowClosedEvent event;
        callback(&event);
    });

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        internalData.width = width;
        internalData.height = height;

        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }

        WindowResizedEvent event(width, height);
        callback(&event);
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }

        MouseMovedEvent event(xpos, ypos);
        callback(&event);
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }
        
        MouseScrolledEvent event(xoffset, yoffset);
        callback(&event);
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event((MouseButton)button, mods);
                callback(&event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event((MouseButton)button, mods);
                callback(&event);
                break;
            }
        }
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(window);
        auto callback = internalData.instance->GetEventCallback();
        if (!callback)
        {
            return;
        }

        switch (action) {
            case GLFW_PRESS:
            case GLFW_REPEAT: {
                KeyPressedEvent event((KeyCode)key, mods);
                callback(&event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event((KeyCode)key, mods);
                callback(&event);
                break;
            }
        }
    });

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return;

    LOG_DEBUG("Successfully loaded OpenGL !\n"
              "            Vendor: %s\n"
              "            Renderer: %s\n"
              "            Version: %s", 
              glGetString(GL_VENDOR), 
              glGetString(GL_RENDERER), 
              glGetString(GL_VERSION));

    // Add GL callbacks
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GlErrorHandler, 0);

    // OpenGL global configuration
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO(); (void) io;
    io.ConfigFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.ConfigFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows

    // UI Styling
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.ScrollbarSize = 12.0f;
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.TabRounding = 4.0f;
    style.GrabRounding = 2.0f;
    style.FramePadding = ImVec2(8, 3);
    style.ItemSpacing = ImVec2(4, 3);
    style.ItemInnerSpacing = ImVec2(3, 4);
    style.IndentSpacing = 12.0f;

    // Set ImGui's font instead of using the rough default one
    Resolver& resolver = Resolver::Get();
    fs::path fontPath = resolver.Resolve("resources/fonts/Roboto-Regular.ttf");
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 15);

    // UI Colors styling
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.13f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.30f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.34f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.38f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.22f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.22f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(1.00f, 1.00f, 1.00f, 0.24f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    colors[ImGuiCol_Separator]              = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.42f, 0.42f, 0.43f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

int Window::GetWidth() const
{
    WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(m_window);
    return internalData.width;
}

int Window::GetHeight() const
{
    WindowInternalData& internalData = *(WindowInternalData*)glfwGetWindowUserPointer(m_window);
    return internalData.height;
}

void Window::Update()
{
    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
    
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::SetTitle(const std::string& title)
{
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::SetEventCallback(const std::function<void(Event *)> &eventCallback)
{
    m_eventCallback = eventCallback;
}

double Window::GetInternalTime() const 
{
    return glfwGetTime();
}

void Window::GLFWErrorHandler(int error, const char* description)
{
    LOG_ERROR("GLFW error %d: %s", error, description);
}

void Window::GlErrorHandler(uint32_t source,
                            uint32_t type,
                            uint32_t id,
                            uint32_t severity,
                            int length,
                            const char* message,
                            const void* userParam )
{
    if (severity == GL_DEBUG_TYPE_ERROR)
        LOG_ERROR("GL error : Type = 0x%x, Severity = 0x%x, Message = %s",
                type, severity, message);

#ifdef DEBUG
    else
        LOG_DEBUG("GL : Type = 0x%x, Severity = 0x%x, Message = %s",
                  type, severity, message);
#endif

}
