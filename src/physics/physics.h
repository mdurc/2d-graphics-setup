#pragma once

#include "../c-lib/dynlist.h"
#include "../c-lib/types.h"

typedef struct {
  fv2 pos;
  fv2 half_size;
} aabb_t;

typedef struct {
  aabb_t aabb;
  fv2 velocity;
  fv2 acceleration;
} body_t;

typedef struct {
  DYNLIST(body_t) body_list;
} physics_state_internal_t;

void physics_init(void);
void physics_update(void);

size_t physics_body_create(fv2 pos, fv2 size);
body_t* physics_body_get(size_t idx);
