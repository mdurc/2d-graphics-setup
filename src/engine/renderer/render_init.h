#pragma once

#include "../c-lib/types.h"
#include "render.h"

void render_init_window(u32 width, u32 height);
void render_init_shaders(u32* shader_program, u32* shader_batch,
                         f32 render_width, f32 render_height);
void render_init_color_texture(u32* texture);

void render_init_quad(u32* vao, u32* vbo, u32* ebo);
void render_init_batch_quads(u32* vao, u32* vbo, u32* ebo);

void render_init_line(u32* vao, u32* vbo);
void render_init_sprite_sheet(sprite_sheet_t* sprite_sheet, const char* path,
                              f32 cell_width, f32 cell_height);
