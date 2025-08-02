#version 330 core
out vec4 frag_color;

in vec4 color;
in vec2 tex_coords;
flat in int texture_slot_index;

uniform sampler2D texture_slots[8]; // set to be 0,1,2,3,4,5,6,7 once

void main() {
  // might have to change this because of: https://stackoverflow.com/a/74729081
  frag_color = texture(texture_slots[texture_slot_index], tex_coords) * color;
}
