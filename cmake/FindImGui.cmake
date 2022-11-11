# FindImGui.cmake
#
# Finds ImGui library
#
# The list of backends to ship with ImGui core can be defined through :
#   ImGui_BACKENDS (default="glfw" "opengl3")
# 
# This will define the following variables :
#   ImGui_FOUND
#   ImGui_ROOT_DIR
#   ImGui_INCLUDE_DIRS
#   ImGui_BACKENDS_DIR
#   ImGui_SOURCES
#   ImGui_DEMO_SOURCES
#   ImGui_VERSION

list(APPEND ImGui_SEARCH_PATH
  ${ImGui_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ImGui
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGui
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGui
)

find_path(ImGui_ROOT_DIR
  NAMES imgui.h
  PATHS ${ImGui_SEARCH_PATH} 
)

if(NOT ImGui_ROOT_DIR)
  message(FATAL_ERROR "ImGui imgui.cpp not found. Set ImGui_DIR to imgui's top-level path (containing \"imgui.cpp\" and \"imgui.h\" files).\n")
else()
  message(VERBOSE "Found possible ImGui root in ${ImGui_ROOT_DIR}")
endif()

set(ImGui_SOURCES
  ${ImGui_ROOT_DIR}/imgui.cpp
  ${ImGui_ROOT_DIR}/imgui_draw.cpp
  ${ImGui_ROOT_DIR}/imgui_tables.cpp
  ${ImGui_ROOT_DIR}/imgui_widgets.cpp
  ${ImGui_ROOT_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(ImGui_DEMO_SOURCES
  ${ImGui_ROOT_DIR}/imgui_demo.cpp
)

# Handle Backends
if(NOT ImGui_BACKENDS)
  set(ImGui_BACKENDS glfw opengl3) 
endif()

set(ImGui_BACKENDS_DIR ${ImGui_ROOT_DIR}/backends)

foreach(imgui_backend ${ImGui_BACKENDS})
  if(EXISTS ${ImGui_BACKENDS_DIR}/imgui_impl_${imgui_backend}.cpp)
    list(APPEND ImGui_SOURCES ${ImGui_BACKENDS_DIR}/imgui_impl_${imgui_backend}.cpp)
    message(VERBOSE "Found backend imgui_impl_${imgui_backend}.cpp")
  else()
    message(WARNING "Could not found backend imgui_impl_${imgui_backend}.cpp inside ${ImGui_BACKENDS_DIR}. This may lead to some compilation errors.")
  endif()
endforeach()

set(ImGui_INCLUDE_DIRS ${ImGui_ROOT_DIR} ${ImGui_BACKENDS_DIR})

# Extract version from header
message( ${ImGui_ROOT_DIR}/imgui.h)
file(
  STRINGS
  ${ImGui_ROOT_DIR}/imgui.h
  ImGui_VERSION
  REGEX "#define I[Mm]G[Uu][Ii]_VERSION "
)
if(NOT ImGui_VERSION)
  message(SEND_ERROR "Can't find version number in ${ImGui_ROOT_DIR}/imgui.h.")
endif()

# Transform '#define ImGui_VERSION "X.Y"' into 'X.Y'
string(REGEX REPLACE ".*\"([0-9.]+).*\".*" "\\1" ImGui_VERSION "${ImGui_VERSION}")

# Check required version
if(${ImGui_VERSION} VERSION_LESS ${ImGui_FIND_VERSION})
  set(ImGui_FOUND FALSE)
  message(FATAL_ERROR "ImGui at with at least v${ImGui_FIND_VERSION} was requested, but only v${ImGui_VERSION} was found")
else()
  set(ImGui_FOUND TRUE)
  message(STATUS "Found ImGui v${ImGui_VERSION} in ${ImGui_ROOT_DIR}")
endif()
