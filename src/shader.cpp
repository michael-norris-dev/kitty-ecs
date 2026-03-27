#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

WGPUShaderModule load_shader_module(WGPUDevice device, const char* filepath) {
  // 1. Open the file
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "Engine Error: Failed to open shader file at " << filepath << std::endl;
    return nullptr;
  }

  // 2. Read the entire file into a string
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string shader_code = buffer.str();

  // 3. Define the WGSL descriptor (specific to wgpu-native v0.2.0 API)
  WGPUShaderModuleWGSLDescriptor wgsl_desc = {};
  wgsl_desc.chain.next = nullptr;
  wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  wgsl_desc.code = shader_code.c_str();

  // 4. Wrap it in the main shader module descriptor
  WGPUShaderModuleDescriptor descriptor = {};
  descriptor.nextInChain = &wgsl_desc.chain;
  descriptor.label = filepath; // Helpful for WebGPU error messages

  // 5. Ask the GPU to compile it
  WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &descriptor);

  if (!shaderModule) {
    std::cerr << "Engine Error: Failed to compile shader module from " << filepath << std::endl;
  }

  return shaderModule;
}
