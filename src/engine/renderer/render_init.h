#pragma once

#include "../c-lib/types.h"
#include "render.h"

void render_init_window(u32 width, u32 height);

// initialize a 1x1 white texture for untextured draws
void render_init_color_texture(u32* texture);
void render_init_shaders(u32* out_shader_2d, u32* out_shader_2d_sprite_batch,
                         u32* out_shader_2d_line_batch, u32* out_shader_3d,
                         f32 render_width, f32 render_height);

// ---- 2d geometry initializers ----
void render_init_quad(u32* vao, u32* vbo, u32* ebo);
void render_init_line(u32* vao, u32* vbo);
void render_init_batch_texture_quads(u32* vao, u32* vbo, u32* ebo);
void render_init_batch_lines(u32* vao, u32* vbo);
void render_init_sprite_sheet(sprite_sheet_t* sprite_sheet, const char* path,
                              f32 cell_width, f32 cell_height);

// ---- 3d geometry initializers ----
void render_init_cube(u32* vao, u32* vbo, u32* ebo);
