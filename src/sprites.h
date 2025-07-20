#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>

#include "c-lib/dynlist.h"
#include "c-lib/types.h"
#include "img.h"

typedef struct {
  int src_x, src_y;
  int dst_x, dst_y;
} sprite_t;

typedef struct {
  img_t img;
  int sprite_width, sprite_height, scale;
  DYNLIST(sprite_t) batch;
} sprite_sheet_t;

void sprites_init(sprite_sheet_t* sheet, state_t* state, const char* path,
                  int sprite_width, int sprite_height);

void font_ch(sprite_sheet_t* font_sheet, char ch, iv2* pos);
void font_str(sprite_sheet_t* font_sheet, const char* str, iv2* pos);

void load_batch(state_t* state, sprite_sheet_t* sheet, bool clr);

#endif
