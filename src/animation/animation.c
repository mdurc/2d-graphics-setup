#include "animation.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/log.h"

static DYNLIST(animation_definition_t) animation_definition_list;
static DYNLIST(animation_t) animation_list;

void animation_init(void) {
  animation_definition_list = dynlist_create(animation_definition_t);
  animation_list = dynlist_create(animation_t);
  LOG("Animation system initialized");
}

void animation_destroy(void) {
  dynlist_destroy(animation_definition_list);
  dynlist_destroy(animation_list);
  LOG("Animation system deinitialized");
}

void animation_deactivate(size_t idx) {
  ASSERT(idx < dynlist_size(animation_list));
  animation_list[idx].is_active = false;
}

size_t animation_definition_count(void) {
  return dynlist_size(animation_definition_list);
}

animation_definition_t* animation_definition_get(size_t idx) {
  ASSERT(idx < dynlist_size(animation_definition_list));
  return &animation_definition_list[idx];
}

size_t animation_definition_create(sprite_sheet_t* sprite_sheet, f32 duration,
                                   u8 row, u8* columns, u8 frame_count) {
  ASSERT(frame_count <= MAX_FRAMES);

  animation_definition_t def = {0};

  def.sprite_sheet = sprite_sheet;
  def.frame_count = frame_count;

  for (u8 i = 0; i < frame_count; ++i) {
    def.frames[i] = (animation_frame_t){
        .column = columns[i],
        .row = row,
        .duration = duration,
    };
  }

  *dynlist_append(animation_definition_list) = def;

  return dynlist_size(animation_definition_list) - 1;
}

size_t animation_count(void) { return dynlist_size(animation_list); }

animation_t* animation_get(size_t idx) {
  ASSERT(idx < dynlist_size(animation_list));
  return &animation_list[idx];
}

size_t animation_create(size_t animation_definition_id, bool does_loop) {
  animation_definition_t* adef =
      &animation_definition_list[animation_definition_id];
  if (adef == NULL) {
    ERROR_EXIT("animation definition with idx %zu not found.",
               animation_definition_id);
  }

  // find a free slot
  size_t size = dynlist_size(animation_list);
  size_t idx = size;
  for (size_t i = 0; i < size /* TODO use dynlist_each instead */; ++i) {
    animation_t* animation = &animation_list[i];
    if (!animation->is_active) {
      idx = i;
      break;
    }
  }

  if (idx == size) {
    *dynlist_append(animation_list) = (animation_t){0};
  }

  animation_t* animation = &animation_list[idx];

  *animation = (animation_t){
      .animation_definition_id = animation_definition_id,
      .current_frame_time = 0.0f,
      .current_frame_index = 0,
      .does_loop = does_loop,
      .is_active = true,
      .is_flipped = false,
  };

  return idx;
}

void animation_update(f32 delta_time) {
  size_t size = dynlist_size(animation_list);
  for (size_t i = 0; i < size; ++i) {
    animation_t* animation = animation_get(i);
    animation_definition_t* adef =
        animation_definition_get(animation->animation_definition_id);

    animation->current_frame_time -= delta_time;

    if (animation->current_frame_time <= 0) {
      animation->current_frame_index += 1;

      // loop or stay on last frame.
      if (animation->current_frame_index == adef->frame_count) {
        if (animation->does_loop) {
          animation->current_frame_index = 0;
        } else {
          animation->current_frame_index -= 1;
        }
      }

      animation->current_frame_time =
          adef->frames[animation->current_frame_index].duration;
    }
  }
}

void animation_render_current_frame(animation_t* animation, vec2 position,
                                    vec2 size, vec4 color) {
  animation_definition_t* adef =
      animation_definition_get(animation->animation_definition_id);
  animation_frame_t* aframe = &adef->frames[animation->current_frame_index];
  render_sprite_sheet_frame(adef->sprite_sheet, aframe->row, aframe->column,
                            position, size, color, animation->is_flipped);
}
