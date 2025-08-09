#pragma once

#include <math.h>

#include "engine/c-lib/types.h"

typedef f32 vec2[2];
typedef f32 vec3[3];
typedef f32 vec4[4];

typedef struct {
  f32 data[16];
} mat4x4;

void vec2_scale(vec2 result, const vec2 a, f32 s);
void vec2_add(vec2 result, const vec2 a, const vec2 b);
void vec2_sub(vec2 result, const vec2 a, const vec2 b);
void vec3_add(vec3 result, const vec3 a, const vec3 b);
void vec3_sub(vec3 result, const vec3 a, const vec3 b);
f32 vec3_len(const vec3 v);
f32 vec3_dot(const vec3 a, const vec3 b);
void vec3_cross(vec3 result, const vec3 a, const vec3 b);
void vec3_normalize(vec3 result, const vec3 v);
void vec3_scale(vec3 result, const vec3 a, f32 s);
void vec4_scale(vec4 result, const vec4 a, f32 s);

// row-major format, so the user should use GL_TRUE in glUniformMatrix transpose
void mat4x4_identity(mat4x4* mat);
void mat4x4_mul(mat4x4* result, const mat4x4* a, const mat4x4* b);
void mat4x4_translate(mat4x4* mat, f32 x, f32 y, f32 z);
void mat4x4_from_translation(mat4x4* mat, f32 x, f32 y, f32 z);
void mat4x4_rotate_x(mat4x4* mat, f32 theta);
void mat4x4_rotate_y(mat4x4* mat, f32 theta);
void mat4x4_rotate_z(mat4x4* mat, f32 theta);
void mat4x4_scale_aniso(mat4x4* mat, f32 sx, f32 sy, f32 sz);
void mat4x4_ortho(mat4x4* mat, f32 left, f32 right, f32 bottom, f32 top,
                  f32 near, f32 far);
void mat4x4_perspective(mat4x4* mat, f32 fov_radians, f32 aspect, f32 n, f32 f);
void mat4x4_look_at(mat4x4* mat, const vec3 eye, const vec3 center,
                    const vec3 up);
