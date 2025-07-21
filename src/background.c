#include "background.h"

#include "state.h"

void push_background(sprite_sheet_t* bg_sheet) {
  int tiles_x = SCREEN_WIDTH / (bg_sheet->scale * bg_sheet->sprite_width);
  int tiles_y = SCREEN_HEIGHT / (bg_sheet->scale * bg_sheet->sprite_height);

  for (int x = 1; x < tiles_x - 1; ++x) {
    for (int y = 1; y < tiles_y - 1; ++y) {
      if ((x != 1 && x != tiles_x - 2) && (y != 1 && y != tiles_y - 2))
        continue;
      *dynlist_append(bg_sheet->batch) =
          (sprite_t){.src_idx = (iv2){x % 2 + y % 2, 0},
                     .dst_px = (fv2){x * bg_sheet->sprite_width,
                                     y * bg_sheet->sprite_height}};
    }
  }
}
