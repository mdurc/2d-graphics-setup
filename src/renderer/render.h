#pragma once

#include "../c-lib/types.h"
#include "linmath/linmath.h"

typedef struct {
  f32 width, height, cell_width, cell_height;
  u32 texture_id;
} sprite_sheet_t;

typedef struct {
  vec2 position;
  vec2 tex_coords;
  vec4 color;
  u32 texture_slot_index;
} batch_vertex_t;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_VERTICES 4 * 10000 // four common vertices per quad, for ebo
#define MAX_BATCH_ELEMENTS 6 * 10000 // one for each corner of the two triangles

void render_init(u32 width, u32 height, f32 scale);
void render_destroy(void);
void render_begin(void);
void render_end(void);

f32 render_get_render_scale(void);
fv2 render_get_render_size(void);

void render_quad(vec2 pos, vec2 size, vec4 color);
void render_quad_lines(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_aabb(f32* aabb, vec4 color);

void render_batch_list(void);
void render_sprite_sheet_frame(sprite_sheet_t* sprite_sheet, f32 row,
                               f32 column, vec2 position, vec2 size, vec4 color,
                               bool is_flipped);

// for testing triangles
void render_test_setup(u32* out_shader, u32* out_vao, f32 scale);
void render_test_triangle(u32 shader, u32 vao);
