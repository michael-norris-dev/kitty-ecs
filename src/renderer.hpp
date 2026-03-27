#pragma once
#include <webgpu/webgpu.h>
#include "primitives.hpp"
#include "window.hpp"

struct Renderer {
  WGPURenderPipeline pipeline;
  WGPUShaderModule shader;
  WGPUBuffer vertexBuffer;
  WGPUBuffer indexBuffer;
  WGPUBuffer instanceBuffer;
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
  int z_index;
};

void initRenderer(Renderer& renderer, GLFWwindow* window, MeshData& mesh);
WGPUTexture load_texture(WGPUDevice device, WGPUQueue queue, const char* filepath);
void destroyRenderer(Renderer& renderer);
