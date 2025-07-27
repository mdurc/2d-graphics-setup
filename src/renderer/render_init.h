#pragma once

#include "../c-lib/types.h"
#include "../sprite_sheet/sprite_sheet.h"

void render_init_window(u32 width, u32 height);
u32 render_create_shader(const char* path_vert, const char* path_frag);
void render_init_color_texture(u32* texture);
void render_init_quad(u32* vao, u32* vbo, u32* ebo);
void render_init_sprite_sheet(sprite_sheet_t* sprite_sheet, const char* path,
                              f32 cell_width, f32 cell_height);
