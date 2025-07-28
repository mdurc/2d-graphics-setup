#version 330 core
out vec4 frag_color;

in vec4 color;
in vec2 tex_coords;

uniform sampler2D texture_slot;

void main() {
  frag_color = texture(texture_slot, tex_coords) * color;
}

