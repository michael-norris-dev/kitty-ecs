#pragma once
#include <webgpu/webgpu.h>

namespace kitty_ecs {
  WGPUShaderModule load_shader_module(WGPUDevice device, const char* filepath);
}
