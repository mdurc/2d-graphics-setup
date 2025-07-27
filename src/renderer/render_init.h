#pragma once

#include "../c-lib/types.h"

void render_init_window(u32 width, u32 height);

u32 render_create_shader(const char* path_vert, const char* path_frag);

void render_init_quad(u32* vao, u32* vbo, u32* ebo);
