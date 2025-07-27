#include "physics.h"

#include "../state.h"

static physics_state_internal_t phys_state;

void physics_init(void) { phys_state.body_list = dynlist_create(body_t); }

void physics_update(void) {
  dynlist_each(phys_state.body_list, body) {
    body->velocity.x += body->acceleration.x * state.time.delta;
    body->velocity.y += body->acceleration.y * state.time.delta;
    body->aabb.pos.x += body->velocity.x * state.time.delta;
    body->aabb.pos.y += body->velocity.y * state.time.delta;
  }
}

size_t physics_body_create(fv2 pos, fv2 size) {
  *dynlist_append(phys_state.body_list) = (body_t){
      .aabb = {.pos = pos, .half_size = {size.x * 0.5f, size.y * 0.5f}},
      .velocity = {0, 0},
      .acceleration = {0, 0}};

  return dynlist_size(phys_state.body_list) - 1;
}

body_t* physics_body_get(size_t idx) {
  ASSERT(idx < (size_t)dynlist_size(phys_state.body_list));
  return &phys_state.body_list[idx];
}
