#include <iostream>
#include <webgpu/webgpu.h>
#include "shader.hpp"
#include "window.hpp"
#include "renderer.hpp"

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
  WGPUBufferDescriptor instBufDesc = {};
  instBufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  instBufDesc.size = sizeof(Instance) * 10000;
  WGPUBuffer instanceBuffer = wgpuDeviceCreateBuffer(device, &instBufDesc);

  // Render Pipeline Setup and Shader Handling
  WGPURenderPipelineDescriptor pipelineDesc = {};

  WGPUShaderModuleWGSLDescriptor wgslDesc = {};
  wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  wgslDesc.code = get_shader_code();

  WGPUShaderModuleDescriptor shaderDesc = {};
  shaderDesc.nextInChain = (const WGPUChainedStruct*)&wgslDesc;
  WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderDesc);

  // Vertex State
  WGPUVertexAttribute vertexAttr = {};
  vertexAttr.format = WGPUVertexFormat_Float32x2;
  vertexAttr.offset = 0;
  vertexAttr.shaderLocation = 0;

  WGPUVertexBufferLayout vertexLayout = {};
  vertexLayout.arrayStride = sizeof(Vertex);
  vertexLayout.stepMode = WGPUVertexStepMode_Vertex;
  vertexLayout.attributeCount = 1;
  vertexLayout.attributes = &vertexAttr;

  WGPUVertexAttribute instanceAttrs[3] = {};
  instanceAttrs[0].format = WGPUVertexFormat_Float32x2; // X, Y offset
  instanceAttrs[0].offset = 0;
  instanceAttrs[0].shaderLocation = 1;
  
  instanceAttrs[1].format = WGPUVertexFormat_Float32x2;
  instanceAttrs[1].offset = 2 * sizeof(float);
  instanceAttrs[1].shaderLocation = 2;
  
  instanceAttrs[2].format = WGPUVertexFormat_Float32x3; // R, G, B color
  instanceAttrs[2].offset = 4 * sizeof(float);
  instanceAttrs[2].shaderLocation = 3;
  
  WGPUVertexBufferLayout instanceLayout = {};
  instanceLayout.arrayStride = sizeof(Instance);
  instanceLayout.stepMode = WGPUVertexStepMode_Instance; // The magic flag!
  instanceLayout.attributeCount = 3;
  instanceLayout.attributes = instanceAttrs;

  WGPUVertexBufferLayout layouts[2] = {vertexLayout, instanceLayout};
  pipelineDesc.vertex.module = shader;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.bufferCount = 2;
  pipelineDesc.vertex.buffers = layouts;

  // Fragment State
  WGPUBlendState blendState = {};
  blendState.color.srcFactor = WGPUBlendFactor_One;
  blendState.color.dstFactor = WGPUBlendFactor_Zero;
  blendState.color.operation = WGPUBlendOperation_Add;
  blendState.alpha.srcFactor = WGPUBlendFactor_One;
  blendState.alpha.dstFactor = WGPUBlendFactor_Zero;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget = {};
  colorTarget.format = surfaceFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask = WGPUColorWriteMask_All;

  WGPUFragmentState fragment = {};
  fragment.module = shader;
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
  renderer.shader = shader;
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
