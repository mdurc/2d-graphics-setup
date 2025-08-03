#include "engine/animation/animation.h"
#include "engine/c-lib/misc.h"
#include "engine/config/config.h"
#include "engine/editor/editor.h"
#include "engine/entity/entity.h"
#include "engine/physics/physics.h"
#include "engine/renderer/render.h"
#include "engine/renderer/render_init.h"
#include "engine/state.h"
#include "engine/time/time.h"

#define CREATE_BLOCK(_x, _y, _w, _h) \
    physics_static_body_create((vec2){_x, _y}, (vec2){_w, _h}, COLLISION_LAYER_TERRAIN);
#define CREATE_STAIRCASE(_x, _y, _size, _dir, _num) \
{ \
  vec2 _pos = {_x, _y}; \
  vec2 _dxv = {(16.0f * _dir), 1.0f}; \
  for (u32 _i = 0; _i < (_num); ++_i) { \
    physics_static_body_create(_pos, _size, COLLISION_LAYER_TERRAIN); \
    vec2_add(_pos, _pos, _dxv); \
  } \
}
#define CREATE_LADDER(_x, _y, _w, _h) \
  physics_body_create((vec2){_x, _y}, (vec2){_w, _h}, (vec2){0, 0}, 0, COLLISION_LAYER_LADDER, ladder_mask, true, ladder_on_hit, ladder_on_hit_static);

// -------- function prototypes --------
void ladder_on_hit(body_t* self, body_t* other, hit_t hit);
void ladder_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void mario_on_hit(body_t* self, body_t* other, hit_t hit);
void mario_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet,
                                 sprite_sheet_t* sprites);
void input_handle(body_t* body);

// -------- global state --------
state_t state;

// -------- collision masks/layers --------
typedef enum {
  COLLISION_LAYER_MARIO = 1,
  COLLISION_LAYER_ENEMY = 1 << 1,
  COLLISION_LAYER_TERRAIN = 1 << 2,
  COLLISION_LAYER_KINEMATIC = 1 << 3,
  COLLISION_LAYER_LADDER = 1 << 4,
} collision_layer_t;

static u8 ladder_mask = COLLISION_LAYER_MARIO;
static u8 mario_mask = COLLISION_LAYER_TERRAIN | COLLISION_LAYER_LADDER;

// -------- physics and animation ids  --------
static size_t mario_eid;
static size_t anim_level;
static size_t anim_mario_idle, anim_mario_walk, anim_mario_jump;
static size_t anim_mario_climb_idle, anim_mario_climb_dyn;

// -------- player game data --------
static const f32 mario_size = 14;
static vec4 mario_aabb_color = {0, 1, 1, 1};
static bool mario_is_grounded = false;
static bool mario_is_flipped = true;
static int mario_move_dir = 0;
static bool mario_is_on_ladder = false;
static bool mario_is_overlapping_ladder = false;

// -------- debug data -------------
static bool is_paused = false;
static bool advance_frame = false;

// -------- collision response callbacks --------
void ladder_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)hit;
  if (other->collision_layer != COLLISION_LAYER_MARIO) {
    return;
  }

  if (mario_is_overlapping_ladder && !mario_is_on_ladder && mario_is_grounded) {
    // check to ENTER ladder mode
    if (state.input.states[INPUT_KEY_UP] > 0 ||
        state.input.states[INPUT_KEY_DOWN] > 0) {
      mario_is_on_ladder = true;
      mario_is_grounded = false;
      other->is_kinematic = true;
      other->collision_mask = COLLISION_LAYER_LADDER;
    }
  }

  if (mario_is_on_ladder) {
    aabb_t* mario_aabb = &other->aabb;
    aabb_t* ladder_aabb = &self->aabb;
    bool at_the_edge = false;

    f32 ladder_top = ladder_aabb->position[1] + ladder_aabb->half_size[1];
    f32 ladder_bottom = ladder_aabb->position[1] - ladder_aabb->half_size[1];

    if (mario_aabb->position[1] >= ladder_top) {
      mario_aabb->position[1] = ladder_top;
      at_the_edge = true;
    }
    if (mario_aabb->position[1] <= ladder_bottom) {
      mario_aabb->position[1] = ladder_bottom;
      at_the_edge = true;
    }

    other->velocity[0] = 0;
    other->velocity[1] = 0;

    if (at_the_edge && (state.input.states[INPUT_KEY_LEFT] ||
                        state.input.states[INPUT_KEY_RIGHT])) {
      mario_is_on_ladder = false;
      other->is_kinematic = false;
      other->collision_mask = mario_mask;
      mario_is_grounded = false;
      return;
    }
    if (state.input.states[INPUT_KEY_UP] > 0) other->velocity[1] = 45;
    if (state.input.states[INPUT_KEY_DOWN] > 0) other->velocity[1] = -45;
  }
}
void ladder_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
}
void mario_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_LADDER) {
    mario_is_overlapping_ladder = true;
  }
}
void mario_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  if (hit.normal[1] > 0) {
    mario_is_grounded = true;
  }
}

// -------- data setup initialization --------
void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet,
                                 sprite_sheet_t* sprites) {
  fv2 render_size = render_get_render_size();

  f32 render_width = render_size.x;
  f32 render_height = render_size.y;
  f32 half_w = render_width * 0.5f;
  f32 half_h = render_height * 0.5f;

  // perimeter
  CREATE_BLOCK(half_w, render_height - 2, render_width - 1, 2);
  CREATE_BLOCK(render_width - 2, half_h, 2, render_height - 1);
  CREATE_BLOCK(half_w, 8, render_width - 1, 2);
  CREATE_BLOCK(2, half_h, 2, render_height - 1);

  mario_eid = entity_create("mario", (vec2){80, 20}, (vec2){9, 9}, (vec2){0, 0},
                            NULL, COLLISION_LAYER_MARIO, mario_mask, false,
                            (size_t)-1, mario_on_hit, mario_on_hit_static);
  *physics_get_terminal_velocity() = -93;
  physics_body_get(entity_get(mario_eid)->body_id)->acceleration[1] = -5.0f;

  // staircase and platforms
  vec2 step_size = (vec2){16, 8};
  CREATE_BLOCK(77, 9, 56 * 2, step_size[1]);
  CREATE_STAIRCASE(141, 10, step_size, 1, 7);
  CREATE_STAIRCASE(221, 37, step_size, -1, 13);
  CREATE_STAIRCASE(45, 37 + 33, step_size, 1, 13);
  CREATE_STAIRCASE(221, 37 + 33 * 2, step_size, -1, 13);
  CREATE_STAIRCASE(45, 37 + 33 * 3, step_size, 1, 13);
  CREATE_STAIRCASE(221, 37 + 33 * 4, step_size, -1, 4);
  CREATE_BLOCK(93, 173, 72 * 2, step_size[1]);
  CREATE_BLOCK(133, 201, 24 * 2, step_size[1]);

  // ladders
  f32 ladder_width = 6;
  CREATE_LADDER(105, 21, ladder_width, 2 * 2);
  CREATE_LADDER(105, 44, 2, 10 * 2);
  CREATE_LADDER(209, 36, ladder_width, 11 * 2);
  CREATE_LADDER(121, 69, ladder_width, 15 * 2);
  CREATE_LADDER(57, 69, ladder_width, 11 * 2);
  CREATE_LADDER(89, 85, ladder_width, 2 * 2);
  CREATE_LADDER(87, 111, 1, 10 * 2);
  CREATE_LADDER(137, 102, ladder_width, 16 * 2);
  CREATE_LADDER(209, 102, ladder_width, 11 * 2);
  CREATE_LADDER(193, 117, ladder_width, 2 * 2);
  CREATE_LADDER(193, 144, ladder_width, 11 * 2);
  CREATE_LADDER(113, 155, ladder_width, 4 * 2);
  CREATE_LADDER(113, 174, ladder_width, 8 * 2);
  CREATE_LADDER(98, 135, 2, 14 * 2);
  CREATE_LADDER(57, 135, ladder_width, 11 * 2);
  CREATE_LADDER(209, 168, ladder_width, 11 * 2);
  CREATE_LADDER(153, 197, ladder_width, 14 * 2);

  size_t anim_level_def = animation_definition_create(
      bg_sheet, (f32[]){1.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f},
      (u8[]){1, 1, 1, 1, 2, 2, 2, 2, 3}, (u8[]){0, 1, 2, 3, 0, 1, 2, 3, 0}, 9);
  anim_level = animation_create(anim_level_def, false);

  size_t anim_mario_idle_def = animation_definition_create(
      sprites, (f32[]){0.1f}, (u8[]){0}, (u8[]){0}, 1);
  anim_mario_idle = animation_create(anim_mario_idle_def, true);

  size_t anim_mario_walk_def = animation_definition_create(
      sprites, (f32[]){0.1f, 0.1f}, (u8[]){0, 0}, (u8[]){1, 2}, 2);
  anim_mario_walk = animation_create(anim_mario_walk_def, true);

  size_t anim_mario_jump_def = animation_definition_create(
      sprites, (f32[]){0.1f}, (u8[]){0}, (u8[]){14}, 1);
  anim_mario_jump = animation_create(anim_mario_jump_def, false);

  size_t anim_mario_climb_idle_def = animation_definition_create(
      sprites, (f32[]){0.1f}, (u8[]){0}, (u8[]){3}, 1);
  anim_mario_climb_idle = animation_create(anim_mario_climb_idle_def, false);

  size_t anim_mario_climb_dyn_def = animation_definition_create(
      sprites, (f32[]){0.1f, 0.1f}, (u8[]){0}, (u8[]){3, 4}, 2);
  anim_mario_climb_dyn = animation_create(anim_mario_climb_dyn_def, true);

  animation_get(anim_mario_idle)->is_flipped = mario_is_flipped;
  animation_get(anim_mario_walk)->is_flipped = mario_is_flipped;
  animation_get(anim_mario_jump)->is_flipped = mario_is_flipped;
  animation_get(anim_mario_climb_idle)->is_flipped = mario_is_flipped;
  animation_get(anim_mario_climb_dyn)->is_flipped = mario_is_flipped;
}

// -------- input handler for the engine input system --------
void input_handle(body_t* body) {
  if (state.input.states[INPUT_KEY_ESCAPE] > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }
  if (!body || mario_is_on_ladder) return;

  body->velocity[0] = 0;
  if (state.input.states[INPUT_KEY_RIGHT] > 0) {
    mario_move_dir = 1;
    body->velocity[0] += 80;
  }
  if (state.input.states[INPUT_KEY_LEFT] > 0) {
    mario_move_dir = -1;
    body->velocity[0] -= 80;
  }
  if (state.input.states[INPUT_KEY_UP] > 0 && mario_is_grounded &&
      !mario_is_overlapping_ladder) {
    body->velocity[1] = 90;
    mario_is_grounded = false;
  }
}

// -------- driver code --------
int main(void) {
  // engine system initialization
  time_init(60);
  config_init();
  render_init(800, 800, 3.0f, BLACK);
  physics_init();
  entity_init();
  animation_init();
  // audio_init();
  editor_init(state.window);

  // -------- Data Setup --------
  sprite_sheet_t bg_sheet;
  render_init_sprite_sheet(&bg_sheet, "res/donkey-kong_bg.png", 224, 256);
  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "res/donkey-kong_font.png", 8, 8);
  sprite_sheet_t sprites;
  render_init_sprite_sheet(&sprites, "res/donkey-kong_sprites.png", 18, 18);

  setup_bodies_entities_anims(&bg_sheet, &sprites);

  // -------- Beginning Game Loop --------
  fv2 render_size = render_get_render_size();
  f32 width = render_size.x;
  f32 height = render_size.y;
  (void)width;
  (void)height;

  while (!glfwWindowShouldClose(state.window)) {
    entity_t* mario = entity_get(mario_eid);
    body_t* mario_body = physics_body_get(mario->body_id);

    // always update the time and input handler
    time_update();
    input_update();

    if (state.input.states[INPUT_KEY_DEBUG] == KS_PRESSED)
      is_paused = !is_paused;
    if (state.input.states[INPUT_KEY_EDITOR_TOGGLE] == KS_PRESSED) {
      editor_toggle_visibility();
      bool is_visible = editor_is_visible();
      glfwSetInputMode(state.window, GLFW_CURSOR,
                       is_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
    editor_update(); // after debug keybind so that editor takes priority

    advance_frame = false;
    if (is_paused && (state.input.states[INPUT_KEY_LEFT] == KS_PRESSED ||
                      state.input.states[INPUT_KEY_RIGHT] == KS_PRESSED ||
                      state.input.states[INPUT_KEY_UP] == KS_PRESSED ||
                      state.input.states[INPUT_KEY_DOWN] == KS_PRESSED)) {
      advance_frame = true;
    }

    if (!is_paused || advance_frame) {
      // if we aren't paused, we can update the physics and animations
      input_handle(mario_body);

      mario_is_overlapping_ladder = false;

      f32 grav = mario_body->acceleration[1];
      if (mario_is_grounded) {
        mario_body->aabb.position[1] += 1.0f;
        mario_body->acceleration[1] = -10000;
      }
      physics_update(state.time.delta);
      mario_body->acceleration[1] = grav;

      animation_update(state.time.delta);

      if (mario_is_on_ladder) {
        mario->animation_id = mario_body->velocity[1] != 0.0f
                                  ? anim_mario_climb_dyn
                                  : anim_mario_climb_idle;
      } else {
        if (!mario_is_grounded) {
          mario->animation_id = anim_mario_jump;
        } else {
          mario->animation_id = mario_body->velocity[0] != 0.0f
                                    ? anim_mario_walk
                                    : anim_mario_idle;
        }
      }
    }

    // --- begin window rendering ---
    render_begin();

    animation_render_current_frame(animation_get(anim_level),
                                   (vec2){width * 0.5f, height * 0.5f}, NULL,
                                   WHITE);

    // render the currently active entity animations from sprite sheet
    for (size_t i = 0; i < entity_count(); ++i) {
      entity_t* entity = entity_get(i);
      if (!entity->is_active || entity->animation_id == (size_t)-1) {
        // it does not have an animation id right now, just skip
        continue;
      }
      body_t* body = physics_body_get(entity->body_id);
      animation_t* anim = animation_get(entity->animation_id);
      if (body->velocity[0] < 0) {
        mario_is_flipped = false;
      } else if (body->velocity[0] > 0) {
        mario_is_flipped = true;
      }
      anim->is_flipped = mario_is_flipped;

      vec2 offset;
      vec2_add(offset, body->aabb.position, (vec2){0, 3});
      animation_render_current_frame(anim, offset,
                                     (vec2){mario_size, mario_size}, WHITE);
    }

    // --- end window rendering ---
    render_sprite_batch(); // batch render the sprite animations/textures

    // render all of the aabbs if we are in debug mode
    if (editor_is_debug()) {
      for (size_t i = 0; i < physics_body_count(); ++i) {
        body_t* body = physics_body_get(i);
        render_aabb((f32*)&body->aabb, WHITE);
      }
      for (size_t i = 0; i < physics_static_body_count(); ++i) {
        static_body_t* body = physics_static_body_get(i);
        render_aabb((f32*)&body->aabb, WHITE);
      }
      render_aabb((f32*)&mario_body->aabb, mario_aabb_color);
    }

    render_aabb_line_batch();

    editor_render(); // render the window last
    render_end();    // glfw swap buffer

    time_update_late();
  }

  // engine system de-initialization
  editor_destroy();
  // audio_destroy();
  animation_destroy();
  entity_destroy();
  physics_destroy();
  render_destroy();
}
