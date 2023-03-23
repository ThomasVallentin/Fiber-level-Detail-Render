cmake_minimum_required(VERSION 3.18)

cmake_policy(SET CMP0048 NEW)
project(FiberLevelDetailRender 
    VERSION 0.0.1 
    LANGUAGES CXX C
)

# General configuration
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3") # define DEBUG macro for debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG") # define DEBUG macro for debug builds

# Third-party
add_subdirectory(third-party)

find_package(ImGui 1.89 REQUIRED)

find_package(OpenMP REQUIRED)

# Configure the executable
file(GLOB_RECURSE SOURCES_FILES "${PROJECT_SOURCE_DIR}/src/**.cpp")
add_executable(FiberLevelDetailRender 
    ${SOURCES_FILES}
    ${ImGui_SOURCES})

target_include_directories(FiberLevelDetailRender PUBLIC 
    src
    ${ImGui_INCLUDE_DIRS})

target_link_libraries(FiberLevelDetailRender 
    glad
    glfw
    glm
    stb
    glimac
    OpenMP::OpenMP_CXX)

target_compile_features(FiberLevelDetailRender PRIVATE cxx_std_17)
