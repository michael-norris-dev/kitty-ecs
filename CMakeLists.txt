cmake_minimum_required(VERSION 3.20)

project(CppWebGpuEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This generates a compile_commands.json file. 
# Point your language server (like clangd) to this, and your editor 
# will have perfect autocomplete for all the GLFW and WebGPU functions!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# 1. Fetch GLFW
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

file(GLOB IMGUI_SOURCES "src/imgui/*.cpp")
file(COPY "src/shaders/shader.wgsl" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
  PackAtlas ALL
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/assets/scripts/atlas_gen.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Stitching raw assets into atlas.png..."
)

# Define our executable
add_executable(engine 
  src/main.cpp 
  src/engine.cpp 
  src/ecs.cpp 
  src/renderer.cpp 
  src/primitives.cpp 
  src/shader.cpp 
  src/window.cpp
  ${IMGUI_SOURCES}
)

add_dependencies(engine PackAtlas)

add_custom_command(
  TARGET engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  ${CMAKE_CURRENT_BINARY_DIR}/assets
)

target_include_directories(engine PRIVATE src/imgui)
target_compile_definitions(engine PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)
# Link both libraries to our executable
target_link_libraries(engine PRIVATE glfw webgpu)

# Copy the wgpu-native dynamic library (.dll or .so) next to the executable 
# so the OS can find it when you actually run the program
target_copy_webgpu_binaries(engine)
