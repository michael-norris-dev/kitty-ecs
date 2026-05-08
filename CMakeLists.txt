cmake_minimum_required(VERSION 3.20)
project(kitty_ecs CXX)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. Fetch the wgpu-native distribution wrapper
set(WEBGPU_BACKEND "WGPU" CACHE STRING "Use wgpu-native" FORCE)

FetchContent_Declare(
  webgpu
  GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution.git
  GIT_TAG main-v0.2.0
)
FetchContent_MakeAvailable(webgpu)

file(GLOB IMGUI_SOURCES "vendor/imgui/*.cpp")

add_library(kitty_ecs 
  src/engine.cpp 
  src/ecs.cpp 
  src/renderer.cpp 
  src/primitives.cpp 
  src/shader.cpp 
  src/window.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(kitty_ecs PUBLIC include)
target_include_directories(kitty_ecs PUBLIC vendor/imgui)
target_compile_definitions(kitty_ecs PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)
target_link_libraries(kitty_ecs PUBLIC glfw webgpu)
