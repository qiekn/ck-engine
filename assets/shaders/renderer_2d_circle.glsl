#type vertex
#version 450 core

layout(location = 0) in vec3 a_world_position;
layout(location = 1) in vec3 a_local_position;
layout(location = 2) in vec4 a_color;
layout(location = 3) in float a_thickness;
layout(location = 4) in float a_fade;
layout(location = 5) in int a_entity_id;

layout(std140, binding = 0) uniform Camera {
  mat4 u_view_projection;
};

struct VertexOutput {
  vec3 local_position;
  vec4 color;
  float thickness;
  float fade;
};

layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_entity_id;

void main() {
  Output.local_position = a_local_position;
  Output.color = a_color;
  Output.thickness = a_thickness;
  Output.fade = a_fade;

  v_entity_id = a_entity_id;

  gl_Position = u_view_projection * vec4(a_world_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_color;
layout(location = 1) out int o_entity_id;

struct VertexOutput {
  vec3 local_position;
  vec4 color;
  float thickness;
  float fade;
};

layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_entity_id;

void main() {
  float fade = max(Input.fade, 0.00001);

  float outer_radius = 1.0;
  float inner_radius = outer_radius - Input.thickness;

  float distance = length(Input.local_position);
  float outer = 1.0 - smoothstep(outer_radius - fade, outer_radius, distance);
  float inner = 1.0 - smoothstep(inner_radius - fade, inner_radius, distance);

  float circle = outer * (1.0 - inner);

  if (circle == 0.0)
    discard;

  o_color = Input.color;
  o_color.a *= circle;

  o_entity_id = v_entity_id;
}
