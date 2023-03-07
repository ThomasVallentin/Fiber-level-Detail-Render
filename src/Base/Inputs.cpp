#include "Inputs.h"

#include "Base/Window.h"


bool Inputs::IsKeyPressed(const KeyCode &key)
{
    int state = glfwGetKey(glfwGetCurrentContext(), (int)key);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Inputs::IsMouseButtonPressed(const MouseButton &button)
{
    int state = glfwGetMouseButton(glfwGetCurrentContext(), (int)button);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

glm::vec2 Inputs::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(glfwGetCurrentContext(), &x, &y);
    return glm::vec2((float)x, (float)y);
}
