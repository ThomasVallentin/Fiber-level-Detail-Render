cmake_minimum_required(VERSION 3.12)

# == GLFW ==
add_subdirectory(glfw)

# == glad ==
add_library(glad glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# == glm ==
add_subdirectory(glm)

# == stb ==
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb/)

# == glimac ==
add_subdirectory(glimac)