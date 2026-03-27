// --- NEW TEXTURE BINDINGS ---
@group(0) @binding(0) var myTexture: texture_2d<f32>;
@group(0) @binding(1) var mySampler: sampler;
// ----------------------------

struct VertexInput {
  @location(0) position: vec2<f32>,
  @location(1) color: vec3<f32>,
  @location(2) uv: vec2<f32>,
};

struct VertexOutput {
  @builtin(position) clip_position: vec4<f32>,
  @location(0) color: vec3<f32>,
  @location(1) uv: vec2<f32>,
};

struct InstanceInput {
  @location(3) instance_pos: vec2<f32>,
  @location(4) instance_scale: vec2<f32>,
  @location(5) instance_color: vec3<f32>,
  @location(6) atlas_offset: vec2<f32>,
};

@vertex
fn vs_main(model: VertexInput, instance: InstanceInput) -> VertexOutput {
  var out: VertexOutput;
  out.color = model.color * instance.instance_color;
  out.uv = (model.uv * 0.25) + instance.atlas_offset;
  let scaled_pos = model.position.xy * instance.instance_scale;
  
  // Pass the raw coordinate directly to the screen
  out.clip_position = vec4<f32>(scaled_pos + instance.instance_pos, 0.0, 1.0); 

  return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
  let tex_color = textureSample(myTexture, mySampler, in.uv);
  return vec4<f32>(in.color * tex_color.rgb, tex_color.a); 
}
