#include "background.h"
#include "c-lib/dynlist.h"
#include "c-lib/time.h"
#include "config/config.h"
#include "font.h"
#include "physics/physics.h"
#include "renderer/render.h"
#include "renderer/render_init.h"
#include "sprite_sheet/img.h"
#include "sprite_sheet/sprite_sheet.h"
#include "state.h"
#include "time.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

static bool should_quit = false;
static fv2 pos;
static input_key_t last_dir = INPUT_KEY_LEFT;
static int animation_idx = 1;

static void input_handle(void) {
  if (state.input.left == KS_PRESSED || state.input.left == KS_HELD) {
    pos.x -= 500 * state.time.delta;
    last_dir = INPUT_KEY_LEFT;
    animation_idx = 1;
  }
  if (state.input.right == KS_PRESSED || state.input.right == KS_HELD) {
    pos.x += 500 * state.time.delta;
    last_dir = INPUT_KEY_RIGHT;
    animation_idx = 2;
  }
  if (state.input.up == KS_PRESSED || state.input.up == KS_HELD) {
    pos.y -= 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 3 : 4;
  }
  if (state.input.down == KS_PRESSED || state.input.down == KS_HELD) {
    pos.y += 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 5 : 6;
  }
  if (state.input.escape == KS_PRESSED || state.input.escape == KS_HELD) {
    should_quit = true;
  }
}

int main(void) {
  // time_init(60);
  // config_init();
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT);
  // physics_init();
  // state_init();

  // int body_count = 500;
  // for (int i = 0; i < body_count; ++i) {
  //   size_t idx = physics_body_create(
  //       (fv2){rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT},
  //       (fv2){rand() % 5, rand() % 5});
  //   body_t* body = physics_body_get(idx);
  //   body->acceleration.x = rand() % 200 - 100;
  //   body->acceleration.y = rand() % 200 - 100;
  // }

  u32 vao_one, vbo_one;
  f32 verts_one[] = {
      // position         // color
      -0.9f, -0.3f, 0.0f, 1.0f, 0.0f, 0.0f, // left
      -0.7f, 0.3f,  0.0f, 0.0f, 1.0f, 0.0f, // top
      -0.5f, -0.3f, 0.0f, 0.0f, 0.0f, 1.0f  // right
  };

  u32 vao_two, vbo_two;
  f32 verts_two[] = {
      // position         // color
      0.9f, -0.3f, 0.0f, 1.0f, 0.0f, 0.0f, // left
      0.7f, 0.3f,  0.0f, 0.0f, 1.0f, 0.0f, // top
      0.5f, -0.3f, 0.0f, 0.0f, 0.0f, 1.0f  // right
  };

  u32 shader_temp = render_create_shader("./src/shaders/temp.vert",
                                         "./src/shaders/temp.frag");
  glGenVertexArrays(1, &vao_one);
  glGenBuffers(1, &vbo_one);
  glBindVertexArray(vao_one);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_one);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts_one), verts_one, GL_STATIC_DRAW);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &vao_two);
  glGenBuffers(1, &vbo_two);
  glBindVertexArray(vao_two);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_two);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts_two), verts_two, GL_STATIC_DRAW);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  pos = (fv2){SCREEN_WIDTH / 2.0f - 100, SCREEN_HEIGHT / 2.0f - 100};

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  while (!glfwWindowShouldClose(state.window)) {
    // time_update();

    // SDL_Event ev;

    if (glfwGetKey(state.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(state.window, true);
    }

    // input_update();
    // input_handle();
    // physics_update();

    // sprite_t player = {.src_idx = (iv2){animation_idx, 0},
    //                    .dst_px = pos,
    //                    .rotation = 0.0f,
    //                    .src_sheet = &state.bg_sheet};

    f32 t = 100.0f + (cosf(time_s()) * 100.0f);

    // DYNLIST(sprite_t) batch = dynlist_create(sprite_t);

    // push_background(&batch, &state.bg_sheet);
    // push_font_ch(&batch, &state.font_sheet, 'A', (fv2){t, t});
    // push_font_str(&batch, &state.font_sheet, msg,
    //               (fv2){t, t + 8.0f * state.font_sheet.scale});

    render_begin();
    glUseProgram(shader_temp);
    render_vao(shader_temp, vao_one);
    render_vao(shader_temp, vao_two);
    render_quad((vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec4){0.0f, 0.0f, 1.0f, 1.0f});
    //   render_batch(&batch, true);
    //   render_sprite(&player, 46.0f);
    //
    //   for (int i = 0; i < body_count; ++i) {
    //     body_t* body = physics_body_get(i);
    //     body->acceleration.x = rand() % 200 - 100;
    //     body->acceleration.y = rand() % 200 - 100;
    //
    //     sprite_sheet_t* bg = &state.bg_sheet;
    //
    //     sprite_t block = {.src_idx = (iv2){6, 6},
    //                       .dst_px = (fv2){body->aabb.pos.x,
    //                       body->aabb.pos.y}, .rotation = 0, .src_sheet = bg};
    //
    //     f32 scale = 3.0f;
    //     render_sprite(&block, scale);
    //
    //     if (body->aabb.pos.x > (SCREEN_WIDTH - bg->sprite_width * scale) ||
    //         body->aabb.pos.x < 0)
    //       body->velocity.x *= -1;
    //     if (body->aabb.pos.y > (SCREEN_HEIGHT - bg->sprite_height * scale) ||
    //         body->aabb.pos.y < 0)
    //       body->velocity.y *= -1;
    //     if (body->velocity.x > 500) body->velocity.x = 500;
    //     if (body->velocity.y > 500) body->velocity.y = 500;
    //     if (body->velocity.x < -500) body->velocity.x = -500;
    //     if (body->velocity.y < -500) body->velocity.y = -500;
    //   }

    render_end();
  }

  state_destroy();
}
