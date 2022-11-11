cmake_policy(SET CMP0048 NEW)
project(OpenGLTest 
    VERSION 0.0.1 
    LANGUAGES CXX C
)

cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

add_subdirectory(third-party/GLFW)
add_subdirectory(third-party/glm)

find_package(ImGui 1.89 REQUIRED)

add_executable(OpenGLTest 
    src/main.cpp 
    src/Renderer/Shader.cpp
    src/Utils/FileUtils.cpp
    third-party/glad/src/glad.c
    ${ImGui_SOURCES})

target_include_directories(OpenGLTest PUBLIC 
    src
    third-party/glad/include
    ${ImGui_INCLUDE_DIRS})

target_link_libraries(OpenGLTest
    glfw
    glm::glm)
