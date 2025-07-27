#pragma once

#include "../c-lib/types.h"
#include "linmath.h"

typedef struct {
  f32 width, height, cell_width, cell_height;
  u32 texture_id;
} sprite_sheet_t;

void render_init(u32 width, u32 height);
void render_begin(void);
void render_end(void);

void render_quad(vec2 pos, vec2 size, vec4 color);
void render_quad_lines(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_aabb(f32* aabb, vec4 color);

// for testing triangles
void render_test_setup(u32* out_shader, u32* out_vao, f32 scale);
void render_test_triangle(u32 shader, u32 vao);
