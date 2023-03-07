#pragma once

#include <GLFW/glfw3.h>

namespace glimac{

/**
 * @brief Returns the time elapsed since the program started, in seconds
 */
inline float getTime() {
    return glfwGetTime();
}
    
} // namespace glimac
