#include "background.h"

#include "state.h"

void add_background(sprite_sheet_t* bg_sheet, iv2 coords[], int num_coords) {
  f32 scale = bg_sheet->scale;
  f32 chars_per_line = SCREEN_WIDTH / (scale * bg_sheet->sprite_width);
  f32 x = 0.0f, y = 0.0f;
  for (int i = 0; i < num_coords; ++i) {
    *dynlist_append(bg_sheet->batch) =
        (sprite_t){.src_idx_x = coords[i].x,
                   .src_idx_y = coords[i].y,
                   .dst_px_x = x * bg_sheet->sprite_width,
                   .dst_px_y = y * bg_sheet->sprite_height};
    ++x;
    if (x >= chars_per_line) {
      ++y;
      x = 0.0f;
    }
  }
}

iv2* generate_coords(int w, int h) {
  iv2* coords = (iv2*)malloc(w * h * sizeof(iv2));
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      iv2* c = &coords[x + y * w];
      c->x = x;
      c->y = y;
    }
  }
  return coords;
}
