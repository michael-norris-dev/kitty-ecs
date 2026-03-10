#include <iostream>
#include <vector>
#include <webgpu/webgpu.h>
#include <algorithm>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "shader.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "primitives.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "ecs.hpp"

void run_engine(int width, int height, const char* title, Application* app) {
  GLFWwindow* window = initWindow(width, height);
  Renderer renderer = {}; 
  MeshData base_shape = generate_quad(0.2f);
  initRenderer(renderer, window, base_shape);  
  Registry registry;
  Input input(window);
  
  app->on_start(registry, renderer);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    app->on_update(registry, input);

    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(renderer.surface, &surfaceTexture);

    WGPUTextureViewDescriptor viewDesc = {};
    viewDesc.format = renderer.surfaceFormat;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);

    WGPUCommandEncoderDescriptor encoderDesc = {};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.device, &encoderDesc);

    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = targetView;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = WGPUColor{ 0.1, 0.2, 0.3, 1.0 };
                                                                
    WGPURenderPassDescriptor passDesc = {};
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
    
    std::vector<Instance> instance_data;
    for (size_t i = 0; i < registry.active_entities.size(); i++) {
      if (registry.active_entities[i]) {
        instance_data.push_back({
          registry.transforms[i].x, registry.transforms[i].y,
          registry.transforms[i].scale_x, registry.transforms[i].scale_y,
          registry.colors[i].r, registry.colors[i].g, registry.colors[i].b,
          registry.transforms[i].z_index
        });
      }
    }
    std::sort(instance_data.begin(), instance_data.end(), [](const Instance& a, const Instance& b) {
      return a.z_index < b.z_index; 
    });

    int data_size = instance_data.size() * sizeof(Instance);
    wgpuQueueWriteBuffer(renderer.queue, renderer.instanceBuffer, 0, instance_data.data(), data_size);
    wgpuRenderPassEncoderSetPipeline(renderPass, renderer.pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, renderer.vertexBuffer, 0, wgpuBufferGetSize(renderer.vertexBuffer));
    wgpuRenderPassEncoderSetIndexBuffer(renderPass, renderer.indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(renderer.indexBuffer));
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 1, renderer.instanceBuffer, 0, data_size);
    wgpuRenderPassEncoderDrawIndexed(renderPass, renderer.indexCount, instance_data.size(), 0, 0, 0);
    wgpuRenderPassEncoderEnd(renderPass);

    WGPUCommandBufferDescriptor cmdBufDesc = {};
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufDesc);
    wgpuQueueSubmit(renderer.queue, 1, &command);

    wgpuSurfacePresent(renderer.surface);

    wgpuCommandBufferRelease(command);
    wgpuRenderPassEncoderRelease(renderPass);
    wgpuCommandEncoderRelease(encoder);
    wgpuTextureViewRelease(targetView);
  }

  destroyRenderer(renderer);
  destroyWindow(window);
}
