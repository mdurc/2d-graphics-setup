#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;
layout (location = 2) in vec4 a_color;
layout (location = 3) in int a_texture_slot_index;

out vec4 color;
out vec2 tex_coords;
flat out int texture_slot_index;

uniform mat4 projection;

void main() {
  color = a_color;
  tex_coords = a_tex_coords;
  texture_slot_index = a_texture_slot_index;
  gl_Position = projection * vec4(a_pos, 0.0f, 1.0f);
}
