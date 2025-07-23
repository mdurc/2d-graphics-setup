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
                  int sprite_width, int sprite_height, f32 scale) {
  load_img(&sheet->img, state, path);
  sheet->sprite_width = sprite_width;
  sheet->sprite_height = sprite_height;
  sheet->batch = dynlist_create(sprite_t);
  sheet->scale = scale;
}

void push_font_ch(sprite_sheet_t* font_sheet, char ch, fv2 dst_px_pos) {
  *dynlist_append(font_sheet->batch) =
      (sprite_t){.src_idx = find_char(ch), .dst_px = dst_px_pos, .rotation = 0};
}

void push_font_str(sprite_sheet_t* font_sheet, const char* str,
                   fv2 dst_px_pos) {
  f32 starting_x = dst_px_pos.x;

  const char* p = str;
  while (*p) {
    if (*p == '\n') {
      dst_px_pos.y += 8.0f;
      dst_px_pos.x = starting_x;
    } else {
      push_font_ch(font_sheet, *p, dst_px_pos);
      dst_px_pos.x += 9.0f;
    }

    ++p;
  }
}

void render_batch(state_t* state, sprite_sheet_t* sheet, bool clr) {
  ASSERT(state && sheet && sheet->batch);
  int bw = sheet->sprite_width;
  int bh = sheet->sprite_height;
  f32 scale = sheet->scale;

  dynlist_each(sheet->batch, sprite) {
    SDL_Rect src = {.x = sprite->src_idx.x * bw,
                    .y = sprite->src_idx.y * bh,
                    .w = bw,
                    .h = bh};
    SDL_Rect dst_px_pos = {.x = (int)(sprite->dst_px.x * scale),
                           .y = (int)(sprite->dst_px.y * scale),
                           .w = (int)(bw * scale),
                           .h = (int)(bh * scale)};
    SDL_RenderCopyEx(state->renderer, sheet->img.texture, &src, &dst_px_pos,
                     sprite->rotation, NULL, SDL_FLIP_NONE);
  }

  if (clr) {
    dynlist_clear(sheet->batch);
  }
}

void destroy_sheet(sprite_sheet_t* sheet) {
  ASSERT(sheet);
  dynlist_destroy(sheet->batch);
  destroy_img(&sheet->img);
}
