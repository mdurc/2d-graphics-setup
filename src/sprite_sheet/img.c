#include "img.h"

#include "../c-lib/misc.h"
#include "../state.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void load_img(img_t* img, const char* path) {
  ASSERT(img, "null arguments in load_img");
  img->pixels = stbi_load(path, &img->width, &img->height, &img->channels, 4);
  ASSERT(img->pixels, "failed to load image from stb image");

  // 4 channels: RGBA, formatted in ABGR8888, 8 bit each (32 bits per pixel)
  SDL_Surface* surface =
      SDL_CreateRGBSurfaceFrom(img->pixels, img->width, img->height, 32,
                               img->width * 4, R_MASK, G_MASK, B_MASK, A_MASK);
  ASSERT(surface, "failed SDL_CreateRGBSurfaceFrom: %s\n", SDL_GetError());

  img->texture = SDL_CreateTextureFromSurface(state.renderer, surface);
  ASSERT(img->texture, "failed to create sdl texture: %s\n", SDL_GetError());
  SDL_FreeSurface(surface);
}

void destroy_img(img_t* img) {
  ASSERT(img && img->pixels && img->texture, "null argument in destroy_img");
  stbi_image_free(img->pixels);
  SDL_DestroyTexture(img->texture);
}
