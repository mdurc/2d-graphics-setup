#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coords; // uv coordinates

out vec2 tex_coords;

uniform mat4 projection;  // camera projection (note that there usually a view matrix that handles the position and orientation of the camera)
uniform mat4 model;       // model that transforms, scales, etc, the current object we are rendering

void main() {
  tex_coords = a_tex_coords;
  gl_Position = projection * model * vec4(a_pos, 1.0f);
}
