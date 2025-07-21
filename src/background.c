#include "background.h"

#include "state.h"

void add_background(sprite_sheet_t* bg_sheet) {
  int tiles_x = SCREEN_WIDTH / (bg_sheet->scale * bg_sheet->sprite_width);
  int tiles_y = SCREEN_HEIGHT / (bg_sheet->scale * bg_sheet->sprite_height);

  for (int y = 1; y < tiles_y - 1; ++y) {
    for (int x = 1; x < tiles_x - 1; ++x) {
      add_sprite(
          bg_sheet, (iv2){x % 2 + y % 2, 0},
          (fv2){x * bg_sheet->sprite_width, y * bg_sheet->sprite_height});
    }
  }
}
