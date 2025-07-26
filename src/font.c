#include "font.h"

#include "c-lib/dynlist.h"
#include "c-lib/misc.h"

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

void push_font_ch(sprite_t** batch, sprite_sheet_t* font_sheet, char ch,
                  fv2 dst_px_pos) {
  *dynlist_append(*batch) = (sprite_t){.src_idx = find_char(ch),
                                       .dst_px = dst_px_pos,
                                       .rotation = 0,
                                       .src_sheet = font_sheet};
}

void push_font_str(sprite_t** batch, sprite_sheet_t* font_sheet,
                   const char* str, fv2 dst_px_pos) {
  f32 starting_x = dst_px_pos.x;
  f32 scale = font_sheet->scale;

  const char* p = str;
  while (*p) {
    if (*p == '\n') {
      dst_px_pos.y += 8.0f * scale;
      dst_px_pos.x = starting_x;
    } else {
      push_font_ch(batch, font_sheet, *p, dst_px_pos);
      dst_px_pos.x += 9.0f * scale;
    }

    ++p;
  }
}
