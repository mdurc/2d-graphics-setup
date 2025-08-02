#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

out vec4 color;

uniform mat4 projection;

void main() {
  color = a_color;
  gl_Position = projection * vec4(a_pos.xy, 0.0, 1.0);
}
