#pragma once

#include "../c-lib/types.h"
#include "../physics/physics.h"
#include "linmath.h"

typedef struct entity {
  size_t body_id;
  size_t animation_id;
  bool is_active;
  u8 health;
} entity_t;

void entity_init(void);
size_t entity_create(vec2 position, vec2 size, vec2 velocity,
                     u8 collision_layer, u8 collision_mask, bool is_kinematic,
                     size_t animation_id, on_hit_func on_hit,
                     on_hit_static_func on_hit_static);
entity_t* entity_get(size_t id);
size_t entity_count(void);
