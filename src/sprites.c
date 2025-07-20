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
  sheet->scale = 10;
}

void font_ch(sprite_sheet_t* font_sheet, char ch, iv2* pos) {
  iv2 texture_src = find_char(ch);
  *dynlist_append(font_sheet->batch) =
      (sprite_t){texture_src.x, texture_src.y, pos->x, pos->y};
}

void font_str(sprite_sheet_t* font_sheet, const char* str, iv2* pos) {
  int starting_x = pos->x;
  int starting_y = pos->y;

  const char* p = str;
  while (*p) {
    if (*p == '\n') {
      pos->y += 8 * font_sheet->scale;
      pos->x = starting_x;
    } else {
      font_ch(font_sheet, *p, pos);
      pos->x += 9 * font_sheet->scale;
    }

    ++p;
  }

  pos->x = starting_x;
  pos->y = starting_y;
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
