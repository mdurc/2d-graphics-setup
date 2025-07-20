#include "sprites.h"

#include "c-lib/dynlist.h"
#include "c-lib/misc.h"
#include "c-lib/types.h"
#include "state.h"

static iv2 find_char(char ch) {
  static const char* lines[] = {
      "abcdefghijklmnop", "qrstuvwxyz",    "ABCDEFGHIJKLMNOP", "QRSTUVWXYZ ",
      "0123456789",       "!@#$%^&*()_+=", ",./<>?;':\"[]"};

  for (size_t y = 0; y < ARRLEN(lines); ++y) {
    size_t x = 0;
    const char* p = lines[y];
    while (*p != '\0') {
      if (*p == ch) {
        return (iv2){x, y};
      }
      ++x;
      ++p;
    }
  }
  return find_char('?');
}

void sprites_init(sprite_sheet_t* sheet, state_t* state, const char* path,
                  int sprite_width, int sprite_height) {
  load_img(&sheet->img, state, path);
  sheet->sprite_width = sprite_width;
  sheet->sprite_height = sprite_height;
  sheet->batch = dynlist_create(sprite_t);

  int chars_per_line = 16;
  int lines = 9;
  int scale_x = SCREEN_WIDTH / (chars_per_line * sprite_width);
  int scale_y = SCREEN_HEIGHT / (lines * sprite_height);
  sheet->scale = min(scale_x, scale_y);
}

void font_ch(sprite_sheet_t* font_sheet, char ch, iv2* dst) {
  iv2 idx = find_char(ch);
  *dynlist_append(font_sheet->batch) = (sprite_t){idx.x, idx.y, dst->x, dst->y};
}

void font_str(sprite_sheet_t* font_sheet, const char* str, iv2* dst) {
  int starting_x = dst->x;
  int starting_y = dst->y;
  int scale = font_sheet->scale;

  const char* p = str;
  while (*p) {
    if (*p == '\n') {
      dst->y += 8 * scale;
      dst->x = starting_x;
    } else {
      font_ch(font_sheet, *p, dst);
      dst->x += 9 * scale;
    }

    ++p;
  }

  dst->x = starting_x;
  dst->y = starting_y;
}

void load_batch(state_t* state, sprite_sheet_t* sheet, bool clr) {
  SDL_SetRenderDrawColor(state->renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(state->renderer);

  int bw = sheet->sprite_width;
  int bh = sheet->sprite_height;
  int scale = sheet->scale;
  dynlist_each(sheet->batch, sprite) {
    SDL_Rect src = {sprite->src_x * bw, sprite->src_y * bh, bw, bh};
    SDL_Rect dst = {sprite->dst_x, sprite->dst_y, bw * scale, bh * scale};
    SDL_RenderCopy(state->renderer, sheet->img.texture, &src, &dst);
  }

  SDL_RenderPresent(state->renderer);

  if (clr) {
    dynlist_clear(sheet->batch);
  }
}
