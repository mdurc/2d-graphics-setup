#include "engine/animation/animation.h"
// #include "engine/audio/audio.h"
#include "engine/c-lib/misc.h"
#include "engine/config/config.h"
#include "engine/editor/editor.h"
#include "engine/entity/entity.h"
#include "engine/font/font.h"
#include "engine/physics/physics.h"
#include "engine/renderer/render.h"
#include "engine/renderer/render_init.h"
#include "engine/state.h"
#include "engine/time/time.h"

// -------- function prototypes --------
void player_on_hit(body_t* self, body_t* other, hit_t hit);
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit(body_t* self, body_t* other, hit_t hit);

void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet);
void input_handle(body_t* body_player);

// -------- global state --------
state_t state;

// -------- collision masks/layers --------
typedef enum {
  COLLISION_LAYER_PLAYER = 1,
  COLLISION_LAYER_ENEMY = 1 << 1,
  COLLISION_LAYER_TERRAIN = 1 << 2,
  COLLISION_LAYER_KINEMATIC = 1 << 3,
} collision_layer_t;

static u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
static u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
static u8 kinematic_mask =
    COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN | COLLISION_LAYER_PLAYER;

// -------- physics and animation ids  --------
static size_t sb_a_id, sb_b_id, sb_c_id, sb_d_id, sb_e_id; // static bodies
static size_t kin_id;                                      // kinematic body
static size_t e_player_id, e_a_id, e_b_id;                 // entities
static size_t anim_player_idle_id, anim_player_walk_id;    // player animations

// -------- player game data --------
static f32 player_size = 36;
static vec4 player_aabb_color = {0, 1, 1, 1};
static bool player_is_grounded = false;

// -------- debug data -------------
static bool is_paused = false;
static bool advance_frame = false;

// -------- collision response callbacks --------
void player_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    player_aabb_color[0] = 1;
    player_aabb_color[2] = 0;
  }
}
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  if (hit.normal[1] > 0) player_is_grounded = true;
}
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)other;
  if (hit.normal[0] > 0) self->velocity[0] = 400;
  if (hit.normal[0] < 0) self->velocity[0] = -400;
}
void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  // LOG("Kinematic body colliding with static body");
}
void kinematic_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_PLAYER) {
    // LOG("Kinematic body colliding with player body");
  } else if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    // LOG("Kinematic body colliding with enemy body");
  }
}

// -------- data setup initialization --------
void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet) {
  fv2 render_size = render_get_render_size();
  f32 render_scale = render_get_render_scale();

  f32 render_width = render_size.x;
  f32 render_height = render_size.y;
  f32 render_half_w = render_width * 0.5f;
  f32 render_half_h = render_height * 0.5f;

  f32 margin = 12;
  f32 thickness = player_size / render_scale;
  f32 player_1_5x = player_size * 1.5f;

  vec2 enemy_size = {player_size * 0.5f, player_size * 0.5f};
  vec2 enemy_vel = {400, 0};

  // static body boundaries
  sb_a_id = physics_static_body_create(
      (vec2){render_half_w, render_height - margin},
      (vec2){render_width - 1, thickness}, COLLISION_LAYER_TERRAIN);
  sb_b_id = physics_static_body_create(
      (vec2){render_width - margin, render_half_h},
      (vec2){thickness, render_height - 1}, COLLISION_LAYER_TERRAIN);
  sb_c_id = physics_static_body_create((vec2){render_half_w, margin},
                                       (vec2){render_width - 1, thickness},
                                       COLLISION_LAYER_TERRAIN);
  sb_d_id = physics_static_body_create((vec2){margin, render_half_h},
                                       (vec2){thickness, render_height - 1},
                                       COLLISION_LAYER_TERRAIN);
  sb_e_id = physics_static_body_create((vec2){render_half_w, render_half_h},
                                       (vec2){player_1_5x, player_1_5x},
                                       COLLISION_LAYER_TERRAIN);

  // kinematic body
  kin_id = physics_body_create((vec2){render_width * 0.85f, render_half_h},
                               (vec2){player_size * 2.0f, player_size * 3.0f},
                               (vec2){-20.f, 0}, 1.0f,
                               COLLISION_LAYER_KINEMATIC, kinematic_mask, true,
                               kinematic_on_hit, kinematic_on_hit_static);

  // entities
  vec2 zero_vel = {0, 0};
  e_player_id = entity_create(
      "player", (vec2){render_half_w - player_size * 3.0f, render_half_h},
      (vec2){player_size, player_size}, zero_vel, 1.0f, COLLISION_LAYER_PLAYER,
      player_mask, false, (size_t)-1, player_on_hit, player_on_hit_static);

  e_a_id =
      entity_create("enemy one", (vec2){render_half_w - 50, render_half_h},
                    enemy_size, enemy_vel, 1.0f, COLLISION_LAYER_ENEMY,
                    enemy_mask, false, (size_t)-1, NULL, enemy_on_hit_static);

  e_b_id =
      entity_create("enemy two", (vec2){render_half_w + 50, render_half_h},
                    enemy_size, enemy_vel, 1.0f, COLLISION_LAYER_ENEMY,
                    enemy_mask, false, (size_t)-1, NULL, enemy_on_hit_static);

  // player animations
  f32* anim_times = (f32[]){0.2f, 0.2f};
  u8* anim_rows = (u8[]){1, 1};
  size_t player_idle_def = animation_definition_create(
      bg_sheet, anim_times, anim_rows, (u8[]){0, 1}, 2);
  size_t player_walk_def = animation_definition_create(
      bg_sheet, anim_times, anim_rows, (u8[]){2, 3}, 2);
  anim_player_idle_id = animation_create(player_idle_def, true);
  anim_player_walk_id = animation_create(player_walk_def, true);
}

// -------- input handler for the engine input system --------
void input_handle(body_t* body_player) {
  if (state.input.escape > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  body_player->velocity[0] = 0;
  if (state.input.right > 0) body_player->velocity[0] += 160;
  if (state.input.left > 0) body_player->velocity[0] -= 160;
  if (state.input.up > 0 && player_is_grounded) {
    body_player->velocity[1] = 1300;
    player_is_grounded = false;
  }
}

// -------- driver code --------
int main(void) {
  // engine system initialization
  time_init(60);
  config_init();
  render_init(1280, 720, 3.0f);
  physics_init();
  entity_init();
  animation_init();
  // audio_init();
  editor_init(state.window);

  // -------- Data Setup --------
  sprite_sheet_t bg_sheet;
  render_init_sprite_sheet(&bg_sheet, "./res/bg.png", 8, 8);
  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "./res/font.png", 8, 8);

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);
  setup_bodies_entities_anims(&bg_sheet);

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  // music_t* drum_music;
  // audio_music_load(&drum_music, "res/drum_music.wav");
  // audio_music_play(drum_music);

  // -------- Beginning Game Loop --------
  fv2 render_size = render_get_render_size();
  f32 width = render_size.x;
  f32 height = render_size.y;
  (void)width;
  (void)height;

  while (!glfwWindowShouldClose(state.window)) {
    // get data from the ids (using ids bc the ptrs could be invalidated)
    entity_t* player = entity_get(e_player_id);
    body_t* body_player = physics_body_get(player->body_id);

    // always update the time and input handler
    time_update();
    input_update();

    if (state.input.debug == KS_PRESSED) is_paused = !is_paused;
    if (state.input.editor_toggle == KS_PRESSED) {
      editor_toggle_visibility();
      bool is_visible = editor_is_visible();
      glfwSetInputMode(state.window, GLFW_CURSOR,
                       is_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
    editor_update(); // after debug keybind so that editor takes priority

    advance_frame = false;
    if (is_paused &&
        (state.input.left == KS_PRESSED || state.input.right == KS_PRESSED ||
         state.input.up == KS_PRESSED || state.input.down == KS_PRESSED)) {
      advance_frame = true;
    }

    if (!is_paused || advance_frame) {
      // if we aren't paused, we can update the physics and animations
      input_handle(body_player);
      physics_update(state.time.delta);
      animation_update(state.time.delta);

      player->animation_id = body_player->velocity[0] != 0.0f
                                 ? anim_player_walk_id
                                 : anim_player_idle_id;
    }

    // --- begin window rendering ---
    render_begin();

    // two triangles
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);

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
      render_aabb((f32*)&body_player->aabb, player_aabb_color); // player body
    }

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
        anim->is_flipped = true;
      } else if (body->velocity[0] > 0) {
        anim->is_flipped = false;
      }
      animation_render_current_frame(anim, body->aabb.position,
                                     (vec2){player_size, player_size}, WHITE);
    }

    // render msg string with font rendering
    float t = 25.0f + (cosf(state.time.now / 1000.0) * 15.0f);
    font_render_str(
        &font_sheet, msg, (vec2){t, height - t},
        (vec2){font_sheet.cell_width * 1.5f, font_sheet.cell_height * 1.5f},
        TURQUOISE);

    // --- end window rendering ---
    render_batch_list(); // batch render the sprite animations/textures
    editor_render();     // render the window last
    render_end();        // glfw swap buffer

    // reset the Player's AABB color to the non-collision color
    player_aabb_color[0] = 0;
    player_aabb_color[2] = 1;

    time_update_late();
  }

  // engine system de-initialization
  // audio_music_destroy(drum_music);
  editor_destroy();
  // audio_destroy();
  animation_destroy();
  entity_destroy();
  physics_destroy();
  render_destroy();
}
