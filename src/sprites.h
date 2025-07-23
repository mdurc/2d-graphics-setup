#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>

#include "c-lib/dynlist.h"
#include "c-lib/types.h"
#include "img.h"

typedef struct {
  iv2 src_idx;  // 2d index position in the sprite sheet texture
  fv2 dst_px;   // target position in the game window
  f32 rotation; // 0, 90, 180, 270
} sprite_t;

typedef struct {
  img_t img;
  int sprite_width, sprite_height;
  f32 scale;
  DYNLIST(sprite_t) batch;
} sprite_sheet_t;

void sprites_init(sprite_sheet_t* sheet, state_t* state, const char* path,
                  int sprite_width, int sprite_height, f32 scale);

void push_font_ch(sprite_sheet_t* font_sheet, char ch, fv2 dst_px_pos);
void push_font_str(sprite_sheet_t* font_sheet, const char* str, fv2 dst_px_pos);

void render_batch(state_t* state, sprite_sheet_t* sheet, bool clr);

void destroy_sheet(sprite_sheet_t* sheet);

#endif
