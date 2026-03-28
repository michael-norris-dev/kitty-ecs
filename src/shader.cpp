#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace kitty_ecs {
  WGPUShaderModule load_shader_module(WGPUDevice device, const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      std::cerr << "Engine Error: Failed to open shader file at " << filepath << std::endl;
      return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string shader_code = buffer.str();

    WGPUShaderModuleWGSLDescriptor wgsl_desc = {};
    wgsl_desc.chain.next = nullptr;
    wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgsl_desc.code = shader_code.c_str();

    WGPUShaderModuleDescriptor descriptor = {};
    descriptor.nextInChain = &wgsl_desc.chain;
    descriptor.label = filepath;

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &descriptor);

    if (!shaderModule) {
      std::cerr << "Engine Error: Failed to compile shader module from " << filepath << std::endl;
    }

    return shaderModule;
  }
}
