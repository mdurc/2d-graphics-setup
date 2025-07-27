#include "physics.h"

#include "../state.h"

static physics_state_internal_t phys_state;

void physics_init(void) { phys_state.body_list = dynlist_create(body_t); }

void physics_update(void) {
  dynlist_each(phys_state.body_list, body) {
    body->velocity[0] += body->acceleration[0] * state.time.delta;
    body->velocity[1] += body->acceleration[1] * state.time.delta;
    body->aabb.position[0] += body->velocity[0] * state.time.delta;
    body->aabb.position[1] += body->velocity[1] * state.time.delta;
  }
}

size_t physics_body_create(vec2 position, vec2 size) {
  *dynlist_append(phys_state.body_list) =
      (body_t){.aabb = {.position = {position[0], position[1]},
                        .half_size = {size[0] * 0.5f, size[1] * 0.5f}},
               .velocity = {0, 0},
               .acceleration = {0, 0}};

  return dynlist_size(phys_state.body_list) - 1;
}

body_t* physics_body_get(size_t idx) {
  ASSERT(idx < (size_t)dynlist_size(phys_state.body_list));
  return &phys_state.body_list[idx];
}

static void aabb_min_max(vec2 min, vec2 max, aabb_t aabb) {
  vec2_sub(min, aabb.position, aabb.half_size);
  vec2_add(max, aabb.position, aabb.half_size);
}

bool physics_point_intersect_aabb(vec2 point, aabb_t aabb) {
  vec2 min, max;
  aabb_min_max(min, max, aabb);
  return point[0] >= min[0] && point[0] <= max[0] && point[1] >= min[1] &&
         point[1] <= max[1];
}
