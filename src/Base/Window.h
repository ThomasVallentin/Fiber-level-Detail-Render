#ifndef WINDOW_H
#define WINDOW_H

#include "Logging.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <functional>
#include <string>


class Event;
class GLFWwindow;
class Window;

static bool s_glfwHasBeenInitialized = false;

struct WindowInternalData {
    uint32_t width;
    uint32_t height;
    std::string title;
    
    Window* instance = nullptr;
};


class Window
{
public:
    explicit Window(const WindowInternalData& settings);
    ~Window();

    int GetWidth() const;
    int GetHeight() const;
    
    void Update();

    void SetTitle(const std::string& title);

    void SetEventCallback(const std::function<void(Event*)>& eventCallback);
    inline std::function<void(Event*)> GetEventCallback() const { return m_eventCallback; };

    inline GLFWwindow* GetInternalWindow() const { return m_window; }
    double GetTime() const;

    bool ShouldClose() const;

private:
    void OnEvent(Event& event);

    static void GLFWErrorHandler(int error, const char* description); 
    static void GlErrorHandler(uint32_t source,
                               uint32_t type,
                               uint32_t id,
                               uint32_t severity,
                               int length,
                               const char* message,
                               const void* userParam); 

    GLFWwindow* m_window;
    WindowInternalData m_internalData;

    std::function<void(Event*)> m_eventCallback;
};


// Utility function to draw aligned fields in the UI

inline void indentedLabel(const std::string& label, const float& indent=120.0f) {
    ImGui::AlignTextToFramePadding();
    float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
    ImGui::SetCursorPosX(indent - labelWidth);
    ImGui::Text(label.c_str());
}


#endif // WINDOW_H