#pragma once
#include <webgpu/webgpu.h>

// Loads a WGSL shader from a text file and compiles it into a WebGPU module
WGPUShaderModule load_shader_module(WGPUDevice device, const char* filepath);
