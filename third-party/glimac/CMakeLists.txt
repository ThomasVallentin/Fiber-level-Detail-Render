cmake_minimum_required(VERSION 3.8)

add_library(glimac)

file(GLOB_RECURSE GLIMAC_SOURCES CONFIGURE_DEPENDS src/*)
target_sources(glimac PRIVATE ${GLIMAC_SOURCES})
target_include_directories(glimac PUBLIC include/)
target_link_libraries(glimac PUBLIC glfw)
target_link_libraries(glimac PUBLIC glad)
target_link_libraries(glimac PUBLIC glm)
