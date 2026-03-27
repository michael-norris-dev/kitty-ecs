#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <webgpu/webgpu.h>
#include "shader.hpp"
#include "window.hpp"
#include "renderer.hpp"

WGPUTexture load_texture(WGPUDevice device, WGPUQueue queue, const char* filepath) {
  // 1. Load the PNG from disk
  int width, height, channels;
  // Force 4 channels (RGBA) so the GPU math is perfectly aligned
  unsigned char* pixels = stbi_load(filepath, &width, &height, &channels, 4); 
  if (!pixels) {
    std::cerr << "Failed to load texture: " << filepath << std::endl;
    return nullptr;
  }

  // 2. Define the GPU Texture
  WGPUExtent3D textureSize = { (uint32_t)width, (uint32_t)height, 1 };
    
  WGPUTextureDescriptor texDesc = {};
  texDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
  texDesc.dimension = WGPUTextureDimension_2D;
  texDesc.size = textureSize;
  texDesc.format = WGPUTextureFormat_RGBA8Unorm; // Standard pixel format
  texDesc.mipLevelCount = 1;
  texDesc.sampleCount = 1;

  WGPUTexture texture = wgpuDeviceCreateTexture(device, &texDesc);

  // 3. Blast the pixels to the GPU Queue
  WGPUImageCopyTexture destination = {};
  destination.texture = texture;

  WGPUTextureDataLayout sourceLayout = {};
  sourceLayout.bytesPerRow = 4 * width; // 4 bytes per pixel (R, G, B, A)
  sourceLayout.rowsPerImage = height;

  wgpuQueueWriteTexture(queue, &destination, pixels, (4 * width * height), &sourceLayout, &textureSize);

  // 4. Free the CPU memory (the GPU has it now!)
  stbi_image_free(pixels);

  return texture;
}

void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata) {
  if (status == WGPURequestAdapterStatus_Success) {
    *static_cast<WGPUAdapter*>(userdata) = adapter;
  } else {
    std::cerr << "Could not get adapter: " << message << "\n";
  }
}

void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* userdata) {
  if (status == WGPURequestDeviceStatus_Success) {
    *static_cast<WGPUDevice*>(userdata) = device;
  } else {
    std::cerr << "Could not get device: " << message << "\n";
  }
}

void initRenderer(Renderer& renderer, GLFWwindow* window, MeshData& mesh) {
  WGPUInstanceDescriptor instDesc = {};
  WGPUInstance instance = wgpuCreateInstance(&instDesc);
  
  WGPUSurface surface = getWindowsSurface(instance, window);

  // Adapter Setup
  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.compatibleSurface = surface;
  WGPUAdapter adapter = nullptr;
  wgpuInstanceRequestAdapter(instance, &adapterOpts, onAdapterRequest, &adapter);

  // Device and Queue Setup
  WGPUDeviceDescriptor deviceDesc = {};
  WGPUDevice device = nullptr;
  wgpuAdapterRequestDevice(adapter, &deviceDesc, onDeviceRequest, &device);
  WGPUQueue queue = wgpuDeviceGetQueue(device);

  // Configuring Surface
  WGPUSurfaceCapabilities surfCaps = {};
  wgpuSurfaceGetCapabilities(surface, adapter, &surfCaps);
  WGPUTextureFormat surfaceFormat = surfCaps.formats[0];

  int width, height;
  getFramebufferSize(window, &width, &height);

  WGPUSurfaceConfiguration config = {};
  config.device = device;
  config.format = surfaceFormat;
  config.usage = WGPUTextureUsage_RenderAttachment;
  config.width = width;
  config.height = height;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;
  wgpuSurfaceConfigure(surface, &config);

  renderer.indexCount = mesh.indices.size();

  // Vertex Buffer Setup
  WGPUBufferDescriptor vBufDesc = {};
  vBufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  vBufDesc.size = mesh.vertices.size() * sizeof(Vertex);
  WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(device, &vBufDesc);
  wgpuQueueWriteBuffer(queue, vertexBuffer, 0, mesh.vertices.data(), vBufDesc.size);
  
  // Index Buffer Setup
  WGPUBufferDescriptor iBufDesc = {};
  iBufDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
  iBufDesc.size = mesh.indices.size() * sizeof(uint16_t);
  WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(device, &iBufDesc);
  wgpuQueueWriteBuffer(queue, indexBuffer, 0, mesh.indices.data(), iBufDesc.size);
  
  // New Instance Buffer
  renderer.instanceCapacity = 10000;
  WGPUBufferDescriptor instBufDesc = {};
  instBufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  instBufDesc.size = sizeof(Instance) * 20000;
  WGPUBuffer instanceBuffer = wgpuDeviceCreateBuffer(device, &instBufDesc);

  // Render Pipeline Setup and Shader Handling
  WGPURenderPipelineDescriptor pipelineDesc = {};
  WGPUShaderModule shader_module = load_shader_module(device, "src/shaders/shader.wgsl");

  if (!shader_module) {
    std::cerr << "CRITICAL FATAL ERROR: Shader module failed to load. " << std::endl;
    exit(1); 
  }

  // Vertex State
  WGPUVertexAttribute vertexAttrs[3] = {};
  vertexAttrs[0].format = WGPUVertexFormat_Float32x2;
  vertexAttrs[0].offset = 0;
  vertexAttrs[0].shaderLocation = 0;

  vertexAttrs[1].format = WGPUVertexFormat_Float32x3;
  vertexAttrs[1].offset = 2 * sizeof(float);
  vertexAttrs[1].shaderLocation = 1;

  vertexAttrs[2].format = WGPUVertexFormat_Float32x2;
  vertexAttrs[2].offset = 5 * sizeof(float);
  vertexAttrs[2].shaderLocation = 2;
  
  WGPUVertexBufferLayout vertexLayout = {};
  vertexLayout.arrayStride = sizeof(Vertex);
  vertexLayout.stepMode = WGPUVertexStepMode_Vertex;
  vertexLayout.attributeCount = 3;
  vertexLayout.attributes = vertexAttrs;

  WGPUVertexAttribute instanceAttrs[4] = {};
  instanceAttrs[0].format = WGPUVertexFormat_Float32x2; // X, Y offset
  instanceAttrs[0].offset = 0;
  instanceAttrs[0].shaderLocation = 3;
  
  instanceAttrs[1].format = WGPUVertexFormat_Float32x2;
  instanceAttrs[1].offset = 2 * sizeof(float);
  instanceAttrs[1].shaderLocation = 4;
  
  instanceAttrs[2].format = WGPUVertexFormat_Float32x3; // R, G, B color
  instanceAttrs[2].offset = 4 * sizeof(float);
  instanceAttrs[2].shaderLocation = 5;
  
  instanceAttrs[3].format = WGPUVertexFormat_Float32x2; // atlas_x, atlas_y
  instanceAttrs[3].offset = 7 * sizeof(float);
  instanceAttrs[3].shaderLocation = 6;
  
  WGPUVertexBufferLayout instanceLayout = {};
  instanceLayout.arrayStride = sizeof(Instance);
  instanceLayout.stepMode = WGPUVertexStepMode_Instance;
  instanceLayout.attributeCount = 4;
  instanceLayout.attributes = instanceAttrs;

  WGPUVertexBufferLayout layouts[2] = {vertexLayout, instanceLayout};
  pipelineDesc.vertex.module = shader_module;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.bufferCount = 2;
  pipelineDesc.vertex.buffers = layouts;

  // Fragment State
  WGPUBlendState blendState = {};
  blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.color.operation = WGPUBlendOperation_Add;
  blendState.alpha.srcFactor = WGPUBlendFactor_One;
  blendState.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget = {};
  colorTarget.format = surfaceFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask = WGPUColorWriteMask_All;

  WGPUFragmentState fragment = {};
  fragment.module = shader_module;
  fragment.entryPoint = "fs_main";
  fragment.targetCount = 1;
  fragment.targets = &colorTarget;
  pipelineDesc.fragment = &fragment;

  // Primitive State
  pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
  pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
  pipelineDesc.primitive.cullMode = WGPUCullMode_Back;

  pipelineDesc.multisample.count = 1;
  pipelineDesc.multisample.mask = ~0u;

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
  
  renderer.pipeline = pipeline;
  renderer.shader = shader_module;
  renderer.vertexBuffer = vertexBuffer;
  renderer.indexBuffer = indexBuffer;
  renderer.instanceBuffer = instanceBuffer;
  renderer.queue = queue;
  renderer.device = device;
  renderer.adapter = adapter;
  renderer.surface = surface;
  renderer.instance = instance;
  renderer.surfaceFormat = surfaceFormat;

  return;
}

void destroyRenderer(Renderer& renderer) {
  wgpuRenderPipelineRelease(renderer.pipeline);
  wgpuShaderModuleRelease(renderer.shader);
  wgpuBufferRelease(renderer.vertexBuffer);
  wgpuBufferRelease(renderer.indexBuffer);
  wgpuBufferRelease(renderer.instanceBuffer);
  wgpuQueueRelease(renderer.queue);
  wgpuDeviceRelease(renderer.device);
  wgpuAdapterRelease(renderer.adapter);
  wgpuSurfaceRelease(renderer.surface);
  wgpuInstanceRelease(renderer.instance);
  
  return;
}
