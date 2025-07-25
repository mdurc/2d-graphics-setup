#include "sprite_sheet.h"

#include "../c-lib/types.h"

void sprite_sheet_init(sprite_sheet_t* sheet, const char* path,
                       int sprite_width, int sprite_height, f32 scale) {
  load_img(&sheet->img, path);
  sheet->sprite_width = sprite_width;
  sheet->sprite_height = sprite_height;
  sheet->scale = scale;
}

void sprite_sheet_destroy(sprite_sheet_t* sheet) {
  if (sheet) destroy_img(&sheet->img);
}
