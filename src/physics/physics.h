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
  bool is_hit;
  f32 time;
  vec2 position;
  vec2 normal;
} hit_t;

typedef struct {
  DYNLIST(body_t) body_list;
} physics_state_internal_t;

void physics_init(void);
void physics_destroy(void);
void physics_update(void);

size_t physics_body_create(vec2 pos, vec2 size);
body_t* physics_body_get(size_t idx);

void physics_aabb_min_max(vec2 min, vec2 max, aabb_t aabb);
bool physics_point_intersect_aabb(vec2 point, aabb_t aabb);
bool physics_aabb_intersect_aabb(aabb_t a, aabb_t b);
aabb_t physics_aabb_minkowski_difference(aabb_t a, aabb_t b);
void physics_aabb_penetration_vector(vec2 r, aabb_t aabb);
hit_t physics_ray_intersect_aabb(vec2 pos, vec2 magnitude, aabb_t aabb);
