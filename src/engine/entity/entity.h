#pragma once

#include "../c-lib/types.h"
#include "../math/math.h"
#include "../physics/physics.h"

typedef struct entity {
  size_t body_id;
  size_t animation_id;
  bool is_active;
  const char* name;
} entity_t;

void entity_init(void);
void entity_destroy(void);
void entity_deactivate(size_t idx);

size_t entity_count(void);
entity_t* entity_get(size_t idx);
const char* entity_get_name(size_t idx);
size_t entity_create(const char* name, vec2 position, vec2 size, vec2 velocity,
                     vec2 acceleration, u8 collision_layer, u8 collision_mask,
                     bool is_kinematic, size_t animation_id, on_hit_func on_hit,
                     on_hit_static_func on_hit_static);
