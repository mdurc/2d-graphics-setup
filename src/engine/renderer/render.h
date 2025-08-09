#pragma once

#include "../c-lib/types.h"
#include "linmath/linmath.h"

typedef struct {
  vec3 position;  // camera position in world space
  vec3 direction; // unit vector camera direction
  vec3 up;        // the "up" direction for the camera (0, 1, 0)

  f32 fov_radians;
  f32 aspect_ratio; // screen width / height
  f32 near_plane;   // near clipping plane
  f32 far_plane;    // far clipping plane
} camera_t; // for 3d rendering, the client is responsible for creating and
            // updating this struct

typedef struct {
  f32 width, height, cell_width, cell_height;
  u32 texture_id;
} sprite_sheet_t;

typedef struct {
  vec2 position;
  vec2 tex_coords;
  vec4 color;
  u32 texture_slot_index;
} batch_sprite_vertex_t;

typedef struct {
  vec2 position;
  vec4 color;
} batch_line_vertex_t;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_LINES MAX_BATCH_QUADS
#define MAX_BATCH_VERTICES 4 * MAX_BATCH_QUADS // four common vertices per quad, for ebo
#define MAX_BATCH_ELEMENTS 6 * MAX_BATCH_QUADS // one for each corner of the two triangles

void render_init(u32 width, u32 height, f32 scale, vec4 bg_color);
void render_destroy(void);

// clears the screen and depth buffer, called once at start of frame
void render_begin(void);
// swaps front and back buffers, called once at end of frame
void render_end(void);

f32 render_get_render_scale(void);
fv2 render_get_render_size(void);

// ---- 2d rendering pass ----
// sets up the orthographic projection, disables depth testing.
void render_begin_2d(void);
// flushes all batched sprites/lines to the screen
void render_sprite_batch(void);
void render_aabb_line_batch(void);
void render_quad(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_quad_lines(vec2 pos, vec2 size, vec4 color);
void render_aabb(f32* aabb, vec4 color);
void render_sprite_sheet_frame(sprite_sheet_t* sprite_sheet, f32 row,
                               f32 column, vec2 position, vec2 size, vec4 color,
                               bool is_flipped);

// ---- 3d rendering pass ----
// sets up the proj and view matrices from the camera and enables depth testing
void render_begin_3d(camera_t* camera);
void render_cube(mat4x4 model, u32* texture_id);

// ---- for testing 2d triangles ----
void render_test_setup(u32* out_shader, u32* out_vao, f32 scale);
void render_test_triangle(u32 shader, u32 vao);
