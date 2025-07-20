#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "c-lib/types.h"
#include "sprites.h"

void add_background(sprite_sheet_t* bg_sheet, iv2 coords[], int num_coords);
iv2* generate_coords(int w, int h);

#endif
