#pragma once

#include "../c-lib/dynlist.h"
#include "linmath.h"

typedef struct {
  vec2 position;
  vec2 half_size;
} aabb_t;

typedef struct {
  aabb_t aabb;
  vec2 velocity;
  vec2 acceleration;
} body_t;

typedef struct {
  DYNLIST(body_t) body_list;
} physics_state_internal_t;

void physics_init(void);
void physics_update(void);

size_t physics_body_create(vec2 pos, vec2 size);
body_t* physics_body_get(size_t idx);
bool physics_point_intersect_aabb(vec2 point, aabb_t aabb);
