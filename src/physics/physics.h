#pragma once

#include "../c-lib/dynlist.h"
#include "linmath.h"

typedef struct body body_t;
typedef struct static_body static_body_t;
typedef struct hit hit_t;

typedef void (*on_hit_func)(body_t* self, body_t* other, hit_t hit);
typedef void (*on_hit_static_func)(body_t* self, static_body_t* other,
                                   hit_t hit);

typedef struct {
  vec2 position;
  vec2 half_size;
} aabb_t;

struct body {
  aabb_t aabb;
  vec2 velocity;
  vec2 acceleration;
  on_hit_func on_hit;
  on_hit_static_func on_hit_static;
  u8 collision_layer;
  u8 collision_mask;
  bool is_kinematic;
  bool is_active;
};

struct static_body {
  aabb_t aabb;
  u8 collision_layer;
};

struct hit {
  size_t other_id;
  f32 time;
  vec2 position;
  vec2 normal;
  bool is_hit;
};

typedef struct {
  f32 gravity, terminal_velocity;

  DYNLIST(body_t) body_list;
  DYNLIST(static_body_t) static_body_list;
} physics_state_internal_t;

void physics_init(void);
void physics_destroy(void);
void physics_body_destroy(size_t id);

void physics_update(void);
void physics_clamp_body(body_t* body);

size_t physics_body_create(vec2 position, vec2 size, vec2 velocity,
                           u8 collision_layer, u8 collision_mask,
                           bool is_kinematic, on_hit_func on_hit,
                           on_hit_static_func on_hit_static);
body_t* physics_body_get(size_t idx);
size_t physics_static_body_create(vec2 position, vec2 size, u8 collision_layer);
static_body_t* physics_static_body_get(size_t idx);

void physics_aabb_min_max(vec2 min, vec2 max, aabb_t aabb);
bool physics_point_intersect_aabb(vec2 point, aabb_t aabb);
bool physics_aabb_intersect_aabb(aabb_t a, aabb_t b);
aabb_t physics_aabb_minkowski_difference(aabb_t a, aabb_t b);
void physics_aabb_penetration_vector(vec2 r, aabb_t aabb);
hit_t physics_ray_intersect_aabb(vec2 pos, vec2 magnitude, aabb_t aabb);
