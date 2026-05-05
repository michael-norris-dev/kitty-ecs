#include <iostream>
#include <vector>
#include <webgpu/webgpu.h> //.h
#include <algorithm>
#include <GLFW/glfw3.h> //.h

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> //.h
#include "memory_metrics.hpp"
#include "scoped_timer.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include "shader.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "primitives.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "ecs.hpp"

void PrintMemoryUsage() {
  std::cout << "\r\033[KCurrent Engine Heap Usage: " << g_Metrics.CurrentUsage() << " bytes";
}

namespace kitty_ecs {
  bool pause_tick = false;

  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)  
      pause_tick = !pause_tick;
  }

  void run_engine(int width, int height, const char* title, Application* app) {
    GLFWwindow* window = initWindow(width, height);
    Renderer renderer = {}; 
    //MeshData base_shape = generate_quad(0.2f);
    MeshData base_shape = generate_hexagon(1.0f);
    initRenderer(renderer, window, base_shape);  
    WGPUTexture main_texture = load_texture(renderer.device, renderer.queue, "assets/atlas.png");
    WGPUTextureViewDescriptor viewDesc = {};
    viewDesc.format = WGPUTextureFormat_RGBA8Unorm;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(main_texture, &viewDesc);

    WGPUSamplerDescriptor samplerDesc = {};
    samplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
    samplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
    samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
    samplerDesc.magFilter = WGPUFilterMode_Nearest;
    samplerDesc.minFilter = WGPUFilterMode_Nearest;
    samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 1.0f;
    samplerDesc.maxAnisotropy = 1;
    WGPUSampler sampler = wgpuDeviceCreateSampler(renderer.device, &samplerDesc);

    WGPUBindGroupEntry bgEntries[3] = {};
    bgEntries[0].binding = 0;
    bgEntries[0].textureView = targetView;
    bgEntries[1].binding = 1;
    bgEntries[1].sampler = sampler;
    bgEntries[2].binding = 2;
    bgEntries[2].buffer = renderer.globalsBuffer;
    bgEntries[2].offset = 0;
    bgEntries[2].size = 64;

    WGPUBindGroupDescriptor bgDesc = {};
    bgDesc.layout = wgpuRenderPipelineGetBindGroupLayout(renderer.pipeline, 0);
    bgDesc.entryCount = 3;
    bgDesc.entries = bgEntries;

    renderer.bindGroup = wgpuDeviceCreateBindGroup(renderer.device, &bgDesc);
    Registry registry;
    Input input(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_Init(renderer.device, 3, renderer.surfaceFormat, WGPUTextureFormat_Undefined);
    glfwSetKeyCallback(window, key_callback);

    app->on_start(registry, renderer);
    const float TIME_PER_TICK = 1.0f / 10.0f;
    float previous_time = glfwGetTime();
    float lag = 0.0f;
    float fps_timer = 0.0f;
    int frame_count = 0;
    float aspect_ratio = 1.0f;
    int current_width = 0;
    int current_height = 0;
    glfwGetFramebufferSize(window, &current_width, &current_height);
    while (!glfwWindowShouldClose(window)) {
      float current_time = glfwGetTime();
      float elapsed_time = current_time - previous_time;
      previous_time = current_time;
      if (!pause_tick) {
        lag += elapsed_time;
      }
      fps_timer += elapsed_time;
      frame_count++;
      glfwPollEvents();
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      if (width == 0 || height == 0) continue;
      if (width != current_width || height != current_height) {
        current_width = width;
        current_height = height;

        WGPUSurfaceConfiguration config = {};
        config.device = renderer.device;
        config.format = renderer.surfaceFormat;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.width = width;
        config.height = height;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        wgpuSurfaceConfigure(renderer.surface, &config);
      }
      ImGui_ImplWGPU_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      app->on_ui(registry);
      ImGui::Render();

      //while (lag >= TIME_PER_TICK && pause_tick == false) {      
      while (lag >= TIME_PER_TICK) {      
        app->on_tick_update(registry, input);
        lag -= TIME_PER_TICK;
      }
      app->on_update(registry, input);
      int w, h;
      glfwGetWindowSize(window, &w, &h);
      float viewport_height = (float)h - 400.0f; // Assuming you have a 400px bottom UI panel
      if (viewport_height < 1.0f) viewport_height = 1.0f;
      float aspect_ratio = (float)w / viewport_height;

      // 2. Fetch the Camera State from the Game
      float cx = app->get_camera_x();
      float cy = app->get_camera_y();
      float zoom = app->get_camera_zoom(); 

      // 3. Create the bounding box around the camera's center point
      float half_w = (zoom * aspect_ratio) / 2.0f;
      float half_h = zoom / 2.0f;

      float left = cx - half_w;
      float right = cx + half_w;
      float bottom = cy - half_h;
      float top = cy + half_h;

      // 4. Construct Column-Major WebGPU Matrix
      float ortho_matrix[16] = {0};
      ortho_matrix[0]  = 2.0f / (right - left);
      ortho_matrix[5]  = 2.0f / (top - bottom);
      ortho_matrix[10] = 1.0f; 
      ortho_matrix[12] = -(right + left) / (right - left); // Translate X
      ortho_matrix[13] = -(top + bottom) / (top - bottom); // Translate Y
      ortho_matrix[15] = 1.0f;

      // Send the camera movement to the GPU!
      wgpuQueueWriteBuffer(renderer.queue, renderer.globalsBuffer, 0, ortho_matrix, 64);

      WGPUSurfaceTexture surfaceTexture;
      wgpuSurfaceGetCurrentTexture(renderer.surface, &surfaceTexture);

      WGPUTextureViewDescriptor screenViewDesc = {};
      screenViewDesc.format = renderer.surfaceFormat; // Must match the window
      screenViewDesc.dimension = WGPUTextureViewDimension_2D;
      screenViewDesc.baseMipLevel = 0;
      screenViewDesc.mipLevelCount = 1;
      screenViewDesc.baseArrayLayer = 0;
      screenViewDesc.arrayLayerCount = 1;
      screenViewDesc.aspect = WGPUTextureAspect_All;

      WGPUTextureView screenView = wgpuTextureCreateView(surfaceTexture.texture, &screenViewDesc);

      WGPUCommandEncoderDescriptor encoderDesc = {};
      WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.device, &encoderDesc);

      WGPURenderPassColorAttachment colorAttachment = {};
      colorAttachment.view = screenView;
      colorAttachment.loadOp = WGPULoadOp_Clear;
      colorAttachment.storeOp = WGPUStoreOp_Store;
      colorAttachment.clearValue = WGPUColor{ 0.1f, 0.2f, 0.3f, 1.0f };

      WGPURenderPassDescriptor passDesc = {};
      passDesc.colorAttachmentCount = 1;
      passDesc.colorAttachments = &colorAttachment;

      WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
      float hud_height = 400.0f;
      wgpuRenderPassEncoderSetViewport(renderPass, 0.0f, 0.0f, (float)width, (float)height - hud_height, 0.0f, 1.0f);

      std::vector<Instance> instance_data;
      for (size_t i = 0; i < registry.active_entities.size(); ++i) {
        if (registry.active_entities[i]) {
          instance_data.push_back({
              registry.transforms[i].x, registry.transforms[i].y,
              registry.transforms[i].scale_x, registry.transforms[i].scale_y,
              registry.colors[i].r, registry.colors[i].g, registry.colors[i].b,
              registry.textures[i].atlas_x, registry.textures[i].atlas_y,
              registry.transforms[i].rotation,
              registry.transforms[i].z_index
              });
        }
      }
      std::sort(instance_data.begin(), instance_data.end(), [](const Instance& a, const Instance& b) {
          return a.z_index < b.z_index; 
          });

      if (instance_data.size() > renderer.instanceCapacity) {
        wgpuBufferRelease(renderer.instanceBuffer);
        renderer.instanceCapacity = instance_data.size() * 1.5f;
        WGPUBufferDescriptor instBufDesc = {};
        instBufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        instBufDesc.size = sizeof(Instance) * renderer.instanceCapacity;
        renderer.instanceBuffer = wgpuDeviceCreateBuffer(renderer.device, &instBufDesc);
      }
      int data_size = instance_data.size() * sizeof(Instance);
      wgpuQueueWriteBuffer(renderer.queue, renderer.instanceBuffer, 0, instance_data.data(), data_size);
      wgpuRenderPassEncoderSetPipeline(renderPass, renderer.pipeline);
      wgpuRenderPassEncoderSetBindGroup(renderPass, 0, renderer.bindGroup, 0, nullptr);
      wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, renderer.vertexBuffer, 0, wgpuBufferGetSize(renderer.vertexBuffer));
      wgpuRenderPassEncoderSetIndexBuffer(renderPass, renderer.indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(renderer.indexBuffer));
      wgpuRenderPassEncoderSetVertexBuffer(renderPass, 1, renderer.instanceBuffer, 0, data_size);
      wgpuRenderPassEncoderDrawIndexed(renderPass, renderer.indexCount, instance_data.size(), 0, 0, 0);
      ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
      wgpuRenderPassEncoderEnd(renderPass);

      WGPUCommandBufferDescriptor cmdBufDesc = {};
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufDesc);
      wgpuQueueSubmit(renderer.queue, 1, &command);

      wgpuSurfacePresent(renderer.surface);

      wgpuCommandBufferRelease(command);
      wgpuRenderPassEncoderRelease(renderPass);
      wgpuCommandEncoderRelease(encoder);
      wgpuTextureViewRelease(screenView);

      if (fps_timer >= 1.0f) {
        // \r and \033[K clear the console line so it updates cleanly in place
        std::cout << "\r\033[K[ FPS: " << frame_count 
                  << " ] | [ Heap: " << g_Metrics.CurrentUsage() << " bytes ]" 
                  << std::flush;
        
        // Optional: Also throw it into the window title!
        std::string title = "Battleship Bebop - FPS: " + std::to_string(frame_count);
        glfwSetWindowTitle(window, title.c_str());

        // Reset the counters for the next second
        frame_count = 0;
        fps_timer = 0.0f;
      }
    }

    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    destroyRenderer(renderer);
    destroyWindow(window);
    }
  }
