#version 330 core
out vec4 frag_color;

in vec2 tex_coords;
in vec3 color;

uniform sampler2D texture_id;

void main() {
  frag_color = texture(texture_id, tex_coords) * vec4(color, 1.0);
}
