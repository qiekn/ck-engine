#type vertex
#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in float a_tex_index;
layout(location = 4) in float a_tiling_factor;
layout(location = 5) in int a_entity_id;

uniform mat4 u_view_projection;
uniform mat4 u_transform;

out vec4 v_color;
out vec2 v_tex_coord;
out flat float v_tex_index;
out float v_tiling_factor;
out flat int v_entity_id;

void main() {
  v_color = a_color;
  v_tex_coord = a_tex_coord;
  v_tex_index = a_tex_index;
  v_tiling_factor = a_tiling_factor;
  v_entity_id = a_entity_id;
  gl_Position = u_view_projection * vec4(a_position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec4 v_color;
in vec2 v_tex_coord;
in flat float v_tex_index;
in float v_tiling_factor;
in flat int v_entity_id;

uniform sampler2D u_textures[32];

void main() {
  color = texture(u_textures[int(v_tex_index)], v_tex_coord * v_tiling_factor) * v_color;
  color2 = v_entity_id;
}
