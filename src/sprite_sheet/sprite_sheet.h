#pragma once

#include <SDL2/SDL.h>

#include "../c-lib/types.h"
#include "img.h"

typedef struct {
  img_t img;
  int sprite_width, sprite_height;
  f32 scale;
} sprite_sheet_t;

typedef struct {
  iv2 src_idx;  // 2d index position in the sprite sheet texture
  fv2 dst_px;   // target position in the game window
  f32 rotation; // 0, 90, 180, 270
  sprite_sheet_t* src_sheet;
} sprite_t;

void sprite_sheet_init(sprite_sheet_t* sheet, const char* path,
                       int sprite_width, int sprite_height, f32 scale);
void sprite_sheet_destroy(sprite_sheet_t* sheet);
