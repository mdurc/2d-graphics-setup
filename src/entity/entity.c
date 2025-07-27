#include "entity.h"

#include "../c-lib/dynlist.h"

static DYNLIST(entity_t) entity_list;

void entity_init(void) { entity_list = dynlist_create(entity_t); }

size_t entity_create(vec2 position, vec2 size, vec2 velocity,
                     u8 collision_layer, u8 collision_mask, on_hit_func on_hit,
                     on_hit_static_func on_hit_static) {
  vec2_scale(size, size, 0.5);

  *dynlist_append(entity_list) = (entity_t){
      .body_id = physics_body_create(position, size, velocity, collision_layer,
                                     collision_mask, on_hit, on_hit_static),
      .is_active = true,
      .health = 20};

  return dynlist_size(entity_list) - 1;
}

entity_t* entity_get(size_t id) {
  ASSERT(id < dynlist_size(entity_list));
  return &entity_list[id];
}

size_t entity_count(void) { return dynlist_size(entity_list); }
