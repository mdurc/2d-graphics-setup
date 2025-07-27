#pragma once

#include <SDL2/SDL.h>

#include "../c-lib/types.h"
#include "../sprite_sheet/sprite_sheet.h"
#include "linmath.h"

void render_init(u32 width, u32 height);
void render_begin(void);
void render_end(void);

void render_vao(u32 prog, u32 vao);
void render_quad(vec2 pos, vec2 size, vec4 color);
void render_sprite(sprite_t* sprite, f32 scale);
void render_batch(sprite_t** batch, bool clear_after_render);
