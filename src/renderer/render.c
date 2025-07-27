#include "render.h"

#include "render_init.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "../c-lib/dynlist.h"
#include "../c-lib/misc.h"
#include "../state.h"

static u32 shader_program;
static u32 vao_quad;
static u32 vbo_quad;
static u32 ebo_quad;

void render_init(u32 width, u32 height) {
  render_init_window(width, height);

  shader_program = render_create_shader("./src/shaders/vert.glsl",
                                        "./src/shaders/frag.glsl");

  glUseProgram(shader_program);
  // every shader/rendering call will now use this program (thus the shaders)

  render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
}

void render_begin(void) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void render_end(void) {
  glfwSwapBuffers(state.window);
  glfwPollEvents();
}

void render_quad(void) {
  glUseProgram(shader_program);

  // the vao stores all of the vbo's linked to it
  // the vao also stores the glBindBuffer calls when the target is
  // GL_ELEMENT_ARRAY, so when we bind this vao, it will also bind the ebo
  // buffer that we associated to it (ebo_quad)
  glBindVertexArray(vao_quad);
  // glDrawArrays(GL_TRIANGLES, 0, 3); // will draw the vertices from the vbo
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // will draw from the ebo

  glBindVertexArray(0);
}

void render_vao(u32 vao) {
  glUseProgram(shader_program);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

void render_sprite(sprite_t* sprite, f32 scale) {}

void render_batch(sprite_t** batch, bool clear_after_render) {
  ASSERT(batch, "dynlist batch must not be null");

  dynlist_each(*batch, sprite) {
    render_sprite(sprite, sprite->src_sheet->scale);
  }

  if (clear_after_render) {
    dynlist_clear(batch);
  }
}
