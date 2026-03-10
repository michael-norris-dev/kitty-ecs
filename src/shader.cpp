#include "shader.hpp"

const char* get_shader_code() { 
  return R"(
    struct VertexInput {
      @location(0) position: vec2<f32>,
    };
    
    struct InstanceInput {
      @location(1) offset: vec2<f32>,
      @location(2) scale: vec2<f32>,
      @location(3) color: vec3<f32>,
    };
    
    struct VertexOutput {
      @builtin(position) clip_position: vec4<f32>,
      @location(0) color: vec3<f32>,
    };

    @vertex 
    fn vs_main(model: VertexInput, instance: InstanceInput) -> VertexOutput {
      var out: VertexOutput;
      let scaled_pos = model.position * instance.scale;
      out.clip_position = vec4<f32>(scaled_pos + instance.offset, 0.0, 1.0);
      
      out.color = instance.color;
      return out;
    }

    @fragment 
    fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
      return vec4<f32>(in.color, 1.0);
    }
  )";
}
