#include "render.h"

#include "render_init.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "../c-lib/dynlist.h"
#include "../c-lib/misc.h"
#include "../state.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static u32 shader_default;
static u32 vao_quad;
static u32 vbo_quad;
static u32 ebo_quad;
static u32 texture_color;

void render_init(u32 width, u32 height) {
  render_init_window(width, height);

  render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
  render_init_color_texture(&texture_color);
  render_init_shaders(&shader_default, width, height);

  stbi_set_flip_vertically_on_load(1);

  glUseProgram(shader_default);
  sprite_sheet_t sheet;
  render_init_sprite_sheet(&sheet, "./res/font.png", 8, 8);
  texture_color = sheet.texture_id;
}

void render_begin(void) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void render_end(void) {
  glfwSwapBuffers(state.window);
  glfwPollEvents();
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
  glUseProgram(shader_default);

  mat4x4 model;
  mat4x4_identity(model);

  mat4x4_translate(model, pos[0], pos[1], 0);
  mat4x4_scale_aniso(model, model, size[0], size[1], 1);

  glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE,
                     &model[0][0]);
  glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

  // the vao stores all of the vbo's linked to it
  // the vao also stores the glBindBuffer calls when the target is
  // GL_ELEMENT_ARRAY, so when we bind this vao, it will also bind the ebo
  // buffer that we associated to it (ebo_quad)
  glBindVertexArray(vao_quad);
  // glDrawArrays(GL_TRIANGLES, 0, 3); // will draw the vertices from the vbo

  glBindTexture(GL_TEXTURE_2D, texture_color);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // will draw from the ebo

  glBindVertexArray(0);
}

void render_vao(u32 prog, u32 vao) {
  glUseProgram(prog);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

void render_sprite(sprite_t* sprite, f32 scale) {}

void render_batch(sprite_t** batch, bool clear_after_render) {
  ASSERT(batch, "dynlist batch must not be null");

  dynlist_each(*batch, sprite) {
    // render_sprite(sprite, sprite->src_sheet->scale);
  }

  if (clear_after_render) {
    dynlist_clear(batch);
  }
}
