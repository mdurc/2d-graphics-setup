#pragma once

#include "../renderer/render.h"

void font_render_char(sprite_sheet_t* font_sheet, char ch, vec2 position,
                      vec2 size, vec4 color);
void font_render_str(sprite_sheet_t* font_sheet, const char* str, vec2 position,
                     vec2 size, vec4 color);
