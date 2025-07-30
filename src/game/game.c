#include "game.h"

#include "../animation/animation.h"
#include "../audio/audio.h"
#include "../c-lib/misc.h"
#include "../entity/entity.h"
#include "../font/font.h"
#include "../renderer/render.h"
#include "../renderer/render_init.h"
#include "../state.h"
#include "../time/time.h"
#include "data.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

void input_handle(body_t* body_player) {
  if (state.input.escape > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  body_player->velocity[0] = 0;
  if (state.input.right > 0) {
    body_player->velocity[0] += 160;
  }
  if (state.input.left > 0) {
    body_player->velocity[0] -= 160;
  }
  if (state.input.up > 0 && player_is_grounded) {
    body_player->velocity[1] = 1300;
    player_is_grounded = false;
  }
}

void run_game_loop(void) {
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

  f32 height = render_get_render_size().y;

  bool is_paused = false;
  bool advance_frame = false;

  music_t* drum_music;
  audio_music_load(&drum_music, "drum_music.wav");
  audio_music_play(drum_music);

  while (!glfwWindowShouldClose(state.window)) {
    entity_t* player = entity_get(e_player_id);
    entity_t* enemy_one = entity_get(e_a_id);
    entity_t* enemy_two = entity_get(e_b_id);
    body_t* body_player = physics_body_get(player->body_id);
    body_t* body_enemy_one = physics_body_get(enemy_one->body_id);
    body_t* body_enemy_two = physics_body_get(enemy_two->body_id);

    // always update the time and input handler
    time_update();
    input_update();

    key_state_t debug = state.input.debug;
    is_paused = (debug == KS_HELD || debug == KS_PRESSED);

    advance_frame = false;
    if (is_paused) {
      // if paused and a movement key is pressed, advance one frame
      if (state.input.left == KS_PRESSED || state.input.right == KS_PRESSED ||
          state.input.up == KS_PRESSED) {
        advance_frame = true;
      }
    }

    if (!is_paused || advance_frame) {
      input_handle(body_player);
      physics_update(state.time.delta);
      animation_update(state.time.delta);

      player->animation_id = body_player->velocity[0] != 0.0f
                                 ? anim_player_walk_id
                                 : anim_player_idle_id;
    }

    static_body_t* sb_a = physics_static_body_get(sb_a_id);
    static_body_t* sb_b = physics_static_body_get(sb_b_id);
    static_body_t* sb_c = physics_static_body_get(sb_c_id);
    static_body_t* sb_d = physics_static_body_get(sb_d_id);
    static_body_t* sb_e = physics_static_body_get(sb_e_id);
    body_t* kin = physics_body_get(kin_id);

    render_begin();

    // two triangles
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);

    // Render all of the aabbs
    render_aabb((f32*)&sb_a->aabb, WHITE);                    // Boundary A
    render_aabb((f32*)&sb_b->aabb, WHITE);                    // Boundary B
    render_aabb((f32*)&sb_c->aabb, WHITE);                    // Boundary C
    render_aabb((f32*)&sb_d->aabb, WHITE);                    // Boundary D
    render_aabb((f32*)&sb_e->aabb, WHITE);                    // Boundary E
    render_aabb((f32*)&kin->aabb, WHITE);                     // Kinematic Block
    render_aabb((f32*)&body_player->aabb, player_aabb_color); // Player body
    render_aabb((f32*)&body_enemy_one->aabb, WHITE);          // enemy one body
    render_aabb((f32*)&body_enemy_two->aabb, WHITE);          // enemy two body

    // Render the currently active entity animations from sprite sheet
    for (size_t i = 0; i < entity_count(); ++i) {
      entity_t* entity = entity_get(i);
      if (entity->animation_id == (size_t)-1) {
        // it does not have an animation id right now
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

    // Render the same message with font rendering
    float t = 25.0f + (cosf(state.time.now / 1000.0) * 15.0f);
    font_render_str(
        &font_sheet, msg, (vec2){t, height - t},
        (vec2){font_sheet.cell_width * 1.5f, font_sheet.cell_height * 1.5f},
        TURQUOISE);

    // batch render the sprite animations/textures
    render_end();

    // reset the Player's AABB color
    player_aabb_color[0] = 0;
    player_aabb_color[2] = 1;

    time_update_late();
  }

  audio_music_destroy(drum_music);
}
