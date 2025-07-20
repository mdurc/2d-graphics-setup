#ifndef IMG_H
#define IMG_H

#include <SDL2/SDL.h>

#include "c-lib/types.h"

typedef struct state state_t;

typedef struct {
  u8* pixels;
  int width, height, channels;
  SDL_Texture* texture;
} img_t;

void load_img(img_t* img, state_t* state, const char* path);
void destroy_img(img_t* img);

#endif
