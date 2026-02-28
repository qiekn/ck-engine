#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in float a_tex_index;
layout(location = 4) in float a_tiling_factor;
layout(location = 5) in int a_entity_id;

layout(std140, binding = 0) uniform Camera {
  mat4 u_view_projection;
};

struct VertexOutput {
  vec4 color;
  vec2 tex_coord;
  float tiling_factor;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat float v_tex_index;
layout(location = 4) out flat int v_entity_id;

void main() {
  Output.color = a_color;
  Output.tex_coord = a_tex_coord;
  Output.tiling_factor = a_tiling_factor;
  v_tex_index = a_tex_index;
  v_entity_id = a_entity_id;

  gl_Position = u_view_projection * vec4(a_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

struct VertexOutput {
  vec4 color;
  vec2 tex_coord;
  float tiling_factor;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat float v_tex_index;
layout(location = 4) in flat int v_entity_id;

layout(binding = 0) uniform sampler2D u_textures[32];

void main() {
  vec4 tex_color = Input.color;

  switch(int(v_tex_index)) {
    case  0: tex_color *= texture(u_textures[ 0], Input.tex_coord * Input.tiling_factor); break;
    case  1: tex_color *= texture(u_textures[ 1], Input.tex_coord * Input.tiling_factor); break;
    case  2: tex_color *= texture(u_textures[ 2], Input.tex_coord * Input.tiling_factor); break;
    case  3: tex_color *= texture(u_textures[ 3], Input.tex_coord * Input.tiling_factor); break;
    case  4: tex_color *= texture(u_textures[ 4], Input.tex_coord * Input.tiling_factor); break;
    case  5: tex_color *= texture(u_textures[ 5], Input.tex_coord * Input.tiling_factor); break;
    case  6: tex_color *= texture(u_textures[ 6], Input.tex_coord * Input.tiling_factor); break;
    case  7: tex_color *= texture(u_textures[ 7], Input.tex_coord * Input.tiling_factor); break;
    case  8: tex_color *= texture(u_textures[ 8], Input.tex_coord * Input.tiling_factor); break;
    case  9: tex_color *= texture(u_textures[ 9], Input.tex_coord * Input.tiling_factor); break;
    case 10: tex_color *= texture(u_textures[10], Input.tex_coord * Input.tiling_factor); break;
    case 11: tex_color *= texture(u_textures[11], Input.tex_coord * Input.tiling_factor); break;
    case 12: tex_color *= texture(u_textures[12], Input.tex_coord * Input.tiling_factor); break;
    case 13: tex_color *= texture(u_textures[13], Input.tex_coord * Input.tiling_factor); break;
    case 14: tex_color *= texture(u_textures[14], Input.tex_coord * Input.tiling_factor); break;
    case 15: tex_color *= texture(u_textures[15], Input.tex_coord * Input.tiling_factor); break;
    case 16: tex_color *= texture(u_textures[16], Input.tex_coord * Input.tiling_factor); break;
    case 17: tex_color *= texture(u_textures[17], Input.tex_coord * Input.tiling_factor); break;
    case 18: tex_color *= texture(u_textures[18], Input.tex_coord * Input.tiling_factor); break;
    case 19: tex_color *= texture(u_textures[19], Input.tex_coord * Input.tiling_factor); break;
    case 20: tex_color *= texture(u_textures[20], Input.tex_coord * Input.tiling_factor); break;
    case 21: tex_color *= texture(u_textures[21], Input.tex_coord * Input.tiling_factor); break;
    case 22: tex_color *= texture(u_textures[22], Input.tex_coord * Input.tiling_factor); break;
    case 23: tex_color *= texture(u_textures[23], Input.tex_coord * Input.tiling_factor); break;
    case 24: tex_color *= texture(u_textures[24], Input.tex_coord * Input.tiling_factor); break;
    case 25: tex_color *= texture(u_textures[25], Input.tex_coord * Input.tiling_factor); break;
    case 26: tex_color *= texture(u_textures[26], Input.tex_coord * Input.tiling_factor); break;
    case 27: tex_color *= texture(u_textures[27], Input.tex_coord * Input.tiling_factor); break;
    case 28: tex_color *= texture(u_textures[28], Input.tex_coord * Input.tiling_factor); break;
    case 29: tex_color *= texture(u_textures[29], Input.tex_coord * Input.tiling_factor); break;
    case 30: tex_color *= texture(u_textures[30], Input.tex_coord * Input.tiling_factor); break;
    case 31: tex_color *= texture(u_textures[31], Input.tex_coord * Input.tiling_factor); break;
  }
  color = tex_color;
  color2 = v_entity_id;
}
