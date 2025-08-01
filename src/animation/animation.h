#pragma once

#include "../c-lib/types.h"
#include "../renderer/render.h"

#define MAX_FRAMES 16

typedef struct {
  f32 duration;
  u8 row, column;
} animation_frame_t;

typedef struct {
  sprite_sheet_t* sprite_sheet;
  animation_frame_t frames[MAX_FRAMES];
  u8 frame_count;
} animation_definition_t;

typedef struct {
  size_t animation_definition_id;
  f32 current_frame_time;
  u8 current_frame_index;
  bool does_loop;
  bool is_active;
  bool is_flipped;
} animation_t;

void animation_init(void);
void animation_destroy(void);
void animation_deactivate(size_t idx);

// instance approach so that each animation can have different frame timing/sync
size_t animation_definition_count(void);
animation_definition_t* animation_definition_get(size_t idx);
size_t animation_definition_create(sprite_sheet_t* sprite_sheet, f32* durations,
                                   u8* rows, u8* columns, u8 frame_count);

size_t animation_count(void);
animation_t* animation_get(size_t idx);
size_t animation_create(size_t animation_definition_id, bool does_loop);

void animation_update(f32 delta_time);
void animation_render_current_frame(animation_t* animation, vec2 position,
                                    vec2 size, vec4 color);
