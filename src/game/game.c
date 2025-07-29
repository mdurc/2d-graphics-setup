#include "game.h"

#include "../animation/animation.h"
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

  f64 x, y;
  glfwGetCursorPos(state.window, &x, &y);

  f32 render_scale = render_get_render_scale();

  f32 velx = 0, vely = body_player->velocity[1];
  if (state.input.right > 0) {
    velx += 500 / render_scale;
  }

  if (state.input.left > 0) {
    velx -= 500 / render_scale;
  }

  if (state.input.up > 0 && player_is_grounded) {
    vely = 4000 / render_scale;
    player_is_grounded = false;
  }

  body_player->velocity[0] = velx;
  body_player->velocity[1] = vely;
}

void run_game_loop(void) {
  sprite_sheet_t bg_sheet;
  render_init_sprite_sheet(&bg_sheet, "./res/bg.png", 8, 8);
  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "./res/font.png", 8, 8);

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);

  setup(&bg_sheet);

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  f32 height = render_get_render_size().y;

  while (!glfwWindowShouldClose(state.window)) {
    time_update();

    entity_t* player = entity_get(entity_player_id);
    body_t* body_player = physics_body_get(player->body_id);

    if (body_player->velocity[0] != 0.0f) {
      player->animation_id = player_anim_walk_id;
    } else {
      player->animation_id = player_anim_idle_id;
    }

    body_t* kinematic_body = physics_body_get(kinematic_body_id);
    static_body_t* static_body_a = physics_static_body_get(static_body_a_id);
    static_body_t* static_body_b = physics_static_body_get(static_body_b_id);
    static_body_t* static_body_c = physics_static_body_get(static_body_c_id);
    static_body_t* static_body_d = physics_static_body_get(static_body_d_id);
    static_body_t* static_body_e = physics_static_body_get(static_body_e_id);

    input_update();
    input_handle(body_player);
    physics_update();
    animation_update(state.time.delta);

    render_begin();

    // two triangles
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);

    render_aabb((f32*)&kinematic_body->aabb, WHITE);
    render_aabb((f32*)&static_body_a->aabb, WHITE);
    render_aabb((f32*)&static_body_b->aabb, WHITE);
    render_aabb((f32*)&static_body_c->aabb, WHITE);
    render_aabb((f32*)&static_body_d->aabb, WHITE);
    render_aabb((f32*)&static_body_e->aabb, WHITE);
    render_aabb((f32*)&body_player->aabb, player_color);

    render_aabb((f32*)physics_body_get(entity_get(entity_a_id)->body_id),
                WHITE);
    render_aabb((f32*)physics_body_get(entity_get(entity_b_id)->body_id),
                WHITE);

    for (size_t i = 0; i < entity_count(); ++i) {
      entity_t* entity = entity_get(i);
      if (entity->animation_id == (size_t)-1) {
        continue;
      }
      body_t* body = physics_body_get(entity->body_id);
      animation_t* anim = animation_get(entity->animation_id);
      if (body->velocity[0] < 0) {
        anim->is_flipped = true;
      } else if (body->velocity[0] > 0) {
        anim->is_flipped = false;
      }
      animation_render(anim, body->aabb.position,
                       (vec2){player_size, player_size}, WHITE);
    }

    float t = 25.0f + (cosf(time_s()) * 15.0f);
    font_render_str(&font_sheet, msg, (vec2){t, height - t}, NULL, TURQUOISE);

    render_end();

    player_color[0] = 0;
    player_color[2] = 1;
    time_update_late();
  }
}
