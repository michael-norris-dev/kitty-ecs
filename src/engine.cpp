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

private bool pause_tick = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)  
    pause_tick = !pause_tick;
}

void run_engine(int width, int height, const char* title, Application* app) {
  GLFWwindow* window = initWindow(width, height);
  Renderer renderer = {}; 
  MeshData base_shape = generate_quad(0.2f);
  initRenderer(renderer, window, base_shape);  
  Registry registry;
  Input input(window);
  glfwSetKeyCallback(window, key_callback);
  
  app->on_start(registry, renderer);
  const double TIME_PER_TICK = 1.0 / 2.0;
  double previous_time = glfwGetTime();
  double lag = 0.0;

  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    double elapsed_time = current_time - previous_time;
    previous_time = current_time;
    lag += elapsed_time;
    glfwPollEvents();
    while (lag >= TIME_PER_TICK && pause_tick == false) {      
      app->on_update(registry, input); // The ECS moves here!       
      lag -= TIME_PER_TICK;     // Drain the bucket
    }
    //app->on_update(registry, input);

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

    if (instance_data.size() > renderer.instanceCapacity) {
      wgpuBufferRelease(renderer.instanceBuffer);
      renderer.instanceCapacity = instance_data.size() * 1.5;
      WGPUBufferDescriptor instBufDesc = {};
      instBufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
      instBufDesc.size = sizeof(Instance) * renderer.instanceCapacity;
      renderer.instanceBuffer = wgpuDeviceCreateBuffer(renderer.device, &instBufDesc);
    }
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
