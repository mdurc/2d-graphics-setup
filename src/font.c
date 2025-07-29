#include "font.h"

#include "c-lib/misc.h"
#include "linmath.h"
#include "renderer/render.h"

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

void font_render_char(sprite_sheet_t* font_sheet, char ch, vec2 position,
                      vec2 size, vec4 color) {
  iv2 tex_idx = find_char(ch);
  render_sprite_sheet_frame(font_sheet, tex_idx.y, tex_idx.x, position, size,
                            color, false);
}

void font_render_str(sprite_sheet_t* font_sheet, const char* str, vec2 position,
                     vec2 size, vec4 color) {
  if (size == NULL) {
    size = (vec2){font_sheet->cell_width, font_sheet->cell_height};
  }

  f32 starting_x = position[0];
  const char* p = str;
  while (*p) {
    if (*p == '\n') {
      position[1] -= size[1];
      position[0] = starting_x;
    } else {
      font_render_char(font_sheet, *p, position, size, color);
      position[0] += size[0];
    }
    ++p;
  }
}
