#include "engine/c-lib/misc.h"
#include "engine/config/config.h"
#include "engine/font/font.h"
#include "engine/renderer/render.h"
#include "engine/renderer/render_init.h"
#include "engine/state.h"
#include "engine/time/time.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

state_t state;

static void input_handle(void) {
  if (state.input.states[INPUT_KEY_ESCAPE] > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }
}

int main(void) {
  time_init(60);
  config_init(); // for initializing input handling
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f, BLACK);

  camera_t camera = {
      .position = {0.0f, 0.0f, 3.0f},
      .direction = {0.0f, 0.0f,
                    -1.0f},       // look towards negative Z (into the screen)
      .up = {0.0f, 1.0f, 0.0f},   // standard up direction
      .fov_radians = M_PI / 4.0f, // 45 degrees field of view
      .aspect_ratio = (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT,
      .near_plane = 0.1f,
      .far_plane = 100.0f};

  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "res/sample_font.png", 8, 8);

  while (!glfwWindowShouldClose(state.window)) {
    time_update();
    input_update();
    input_handle();

    render_begin();
    render_begin_3d(&camera);
    {
      mat4x4 model_matrix;
      mat4x4_identity(&model_matrix);
      mat4x4_scale_aniso(&model_matrix, 1.0f, 1.0f, 1.0f);
      float rotation_angle = (float)glfwGetTime();
      mat4x4_rotate_y(&model_matrix, rotation_angle);
      mat4x4_rotate_x(&model_matrix, rotation_angle);
      vec3 cube_position = {0.75f, 0.0f, 0.0f};
      mat4x4_translate(&model_matrix, cube_position[0], cube_position[1],
                       cube_position[2]);
      render_cube(&model_matrix, NULL);
    }

    render_begin_2d();
    {
      font_render_str(&font_sheet, "3d cube", (vec2){30, SCREEN_HEIGHT - 30},
                      (vec2){35, 35}, TURQUOISE);
      render_sprite_batch();
    }
    render_end();

    time_update_late();
  }

  render_destroy();
}
