#pragma once
#include <webgpu/webgpu.h>
#include "primitives.hpp"
#include "window.hpp"

namespace kitty_ecs {
  struct Renderer {
    WGPURenderPipeline pipeline;
    WGPUShaderModule shader;
    WGPUBuffer vertexBuffer;
    WGPUBuffer indexBuffer;
    WGPUBuffer instanceBuffer;
    WGPUBuffer globalsBuffer;
    WGPUQueue queue;
    WGPUDevice device;
    WGPUAdapter adapter;
    WGPUSurface surface;
    WGPUInstance instance;
    WGPUTextureFormat surfaceFormat;
    WGPUBindGroup bindGroup;
    size_t instanceCapacity;
    int indexCount;
  };

  struct Instance {
    float x, y;
    float scale_x, scale_y;
    float r, g, b;
    float atlas_x, atlas_y;
    float rotation;
    int z_index;
  };

  void initRenderer(Renderer& renderer, GLFWwindow* window, MeshData& mesh);
  WGPUTexture load_texture(WGPUDevice device, WGPUQueue queue, const char* filepath);
  void destroyRenderer(Renderer& renderer);
}
