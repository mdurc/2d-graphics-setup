#include "entity.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/log.h"

static DYNLIST(entity_t) entity_list;

void entity_init(void) {
  entity_list = dynlist_create(entity_t);
  LOG("Entity system initialized");
}
void entity_destroy(void) {
  dynlist_destroy(entity_list);
  LOG("Entity system deinitialized");
}

void entity_deactivate(size_t idx) {
  entity_t* entity = entity_get(idx);
  entity->is_active = false;
  physics_deactivate(entity->body_id);
}

size_t entity_count(void) { return dynlist_size(entity_list); }

entity_t* entity_get(size_t idx) {
  ASSERT(idx < dynlist_size(entity_list));
  return &entity_list[idx];
}

const char* entity_get_name(size_t idx) { return entity_get(idx)->name; }

size_t entity_create(const char* name, vec2 position, vec2 size, vec2 velocity,
                     f32 gravity_scale, u8 collision_layer, u8 collision_mask,
                     bool is_kinematic, size_t animation_id, on_hit_func on_hit,
                     on_hit_static_func on_hit_static) {
  size_t list_size = dynlist_size(entity_list);
  size_t id = list_size;
  for (size_t i = 0; i < list_size; ++i) {
    entity_t* entity = &entity_list[i];
    if (!entity->is_active) {
      id = i;
      break;
    }
  }

  if (id == list_size) {
    *dynlist_append(entity_list) = (entity_t){0};
  }

  entity_t* entity = entity_get(id);

  *entity = (entity_t){
      .body_id = physics_body_create(position, size, velocity, gravity_scale,
                                     collision_layer, collision_mask,
                                     is_kinematic, on_hit, on_hit_static),
      .animation_id = animation_id,
      .is_active = true,
      .name = name,
  };

  return id;
}
