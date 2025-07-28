#include "render_init.h"

#include "../c-lib/misc.h"
#include "../io/io.h"
#include "../state.h"
#include "linmath.h"
#include "stb_image.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>

static void framebuffer_size_callback(GLFWwindow* window, int width,
                                      int height) {
  ASSERT(window);
  glViewport(0, 0, width, height);
}

void render_init_window(u32 width, u32 height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  state.window = glfwCreateWindow(width, height, "window", NULL, NULL);
  if (state.window == NULL) {
    glfwTerminate();
    ERROR_RETURN(, "failed to create glfw window\n");
  }
  glfwMakeContextCurrent(state.window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    ERROR_RETURN(, "failed to initialize glad\n");
  }

  int framebuffer_width, framebuffer_height;
  glfwGetFramebufferSize(state.window, &framebuffer_width, &framebuffer_height);
  glViewport(0, 0, framebuffer_width, framebuffer_height);
  glfwSetFramebufferSizeCallback(state.window, framebuffer_size_callback);

  glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  printf("OpenGL Loaded\n");
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));

  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum nr of vertex attributes supported: %d\n", nrAttributes);
}

static u32 render_create_shader(const char* path_vert, const char* path_frag) {
  int success;
  char log[512];

  file_t file_vert = io_file_read(path_vert);
  ASSERT(file_vert.is_valid, "error reading vertex shader file\n");
  u32 shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader_vertex, 1, (const char* const*)&file_vert.data, NULL);
  glCompileShader(shader_vertex);
  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_vertex, 512, NULL, log);
    ERROR_EXIT("error compiling vertex shader. %s\n", log);
  }

  file_t file_frag = io_file_read(path_frag);
  ASSERT(file_frag.is_valid, "error reading fragment shader file\n");
  u32 shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment, 1, (const char* const*)&file_frag.data, NULL);
  glCompileShader(shader_fragment);
  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_fragment, 512, NULL, log);
    ERROR_EXIT("error compiling fragment shader. %s\n", log);
  }

  u32 shader_program = glCreateProgram();
  glAttachShader(shader_program, shader_vertex);
  glAttachShader(shader_program, shader_fragment);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, log);
    ERROR_EXIT("error linking shader. %s\n", log);
  }

  // we can delete the shaders now because we have linked them
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);

  free(file_vert.data);
  free(file_frag.data);

  return shader_program;
}

void render_init_shaders(u32* shader_program, u32* shader_batch,
                         f32 render_width, f32 render_height) {
  mat4x4 projection;
  *shader_program = render_create_shader("./src/shaders/vert.glsl",
                                         "./src/shaders/frag.glsl");
  *shader_batch = render_create_shader("./src/shaders/batch_vert.glsl",
                                       "./src/shaders/batch_frag.glsl");

  // orthographic camera view to get the pixel size we want, and applying the
  // projection to the entire window.
  mat4x4_ortho(projection, 0, render_width, 0, render_height, -2, 2);

  glUseProgram(*shader_program);
  glUniformMatrix4fv(glGetUniformLocation(*shader_program, "projection"), 1,
                     GL_FALSE, &projection[0][0]);

  glUseProgram(*shader_batch);
  glUniformMatrix4fv(glGetUniformLocation(*shader_batch, "projection"), 1,
                     GL_FALSE, &projection[0][0]);
}

void render_init_color_texture(u32* texture) {
  // this will be the blank texture so that whatever color we wish to draw to
  // the object, it will be that color only
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);

  u8 solid_white[4] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               solid_white);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void render_init_quad(u32* vao, u32* vbo, u32* ebo) {
  // when drawing two triangles, we will have 2 extra vertices, which is not ok.
  // EBO, element buffer objects select a specific order of vertices to use.

  // only 4 vertices here instead of 6
  f32 vertices[] = {
      // position         // texture coordinates (uv)
      0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 0.0f, 1.0f  // top left
  };

  u32 idxs[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  glGenVertexArrays(1, vao);
  glGenBuffers(1, vbo);
  glGenBuffers(1, ebo);

  glBindVertexArray(*vao);

  // VBO, vertex buffer objects store the vertex data in the GPU memory
  glBindBuffer(GL_ARRAY_BUFFER, *vbo); // GL_ARRAY_BUFFER now linked to VBO obj
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
  glEnableVertexAttribArray(0);

  // uv texture coordinate attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // do NOT unbind the EBO while a VAO is active as the bound element
  // buffer object IS stored in the VAO; keep the EBO bound.
  glBindVertexArray(0);                     // unbind vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);         // unbind vbo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // for wireframe mode
}

void render_init_batch_quads(u32* vao, u32* vbo, u32* ebo) {
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  // each element is an index for the ebo buffer
  // in one quad: 4 required vertices (corners of the quad), 6 required
  // element/indices (3 for both triangles).
  // since each quad will always have 6 indices, we can simply fill in the index
  // array for the ebo with the desired offsets, which are the vertex offsets.
  u32 indices[MAX_BATCH_ELEMENTS];
  for (u32 i = 0, offset = 0; i < MAX_BATCH_ELEMENTS; i += 6, offset += 4) {
    // triangle one
    indices[i + 0] = offset + 0; // top left
    indices[i + 1] = offset + 1; // top right
    indices[i + 2] = offset + 2; // bottom right
    // triangle two
    indices[i + 3] = offset + 2; // bottom right
    indices[i + 4] = offset + 3; // bottom left
    indices[i + 5] = offset + 0; // top left
  }

  glGenBuffers(1, vbo);
  glBindBuffer(GL_ARRAY_BUFFER, *vbo);
  glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(batch_vertex_t),
               NULL, GL_DYNAMIC_DRAW);
  // vbo data is NULL, as we will be updating the contents at each frame,
  // thus we want touse GL_DYNAMIC_DRAW, just like with the line segments

  // [x, y], [u, v], [r, g, b, a]
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(batch_vertex_t),
                        (void*)offsetof(batch_vertex_t, position));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(batch_vertex_t),
                        (void*)offsetof(batch_vertex_t, tex_coords));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(batch_vertex_t),
                        (void*)offsetof(batch_vertex_t, color));

  glGenBuffers(1, ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BATCH_ELEMENTS * sizeof(u32),
               indices, GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render_init_line(u32* vao, u32* vbo) {
  glGenVertexArrays(1, vao);
  glGenBuffers(1, vbo);

  glBindVertexArray(*vao);
  glBindBuffer(GL_ARRAY_BUFFER, *vbo);
  // specifying the size, but not providing any data yet, as it will be updating
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
  glEnableVertexAttribArray(0);
  // we do not need to add the uv texture coordinate attribute

  glBindVertexArray(0);             // unbind the vao
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the vbo
}

void render_init_sprite_sheet(sprite_sheet_t* sprite_sheet, const char* path,
                              f32 cell_width, f32 cell_height) {
  glGenTextures(1, &sprite_sheet->texture_id);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sprite_sheet->texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // nearest for pixel art, GL_LINEAR for smoother textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  int width, height, channel_count;
  u8* image_data = stbi_load(path, &width, &height, &channel_count, 0);
  ASSERT(image_data, "failed to load image from stb image: %s", path);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  sprite_sheet->width = (f32)width;
  sprite_sheet->height = (f32)height;
  sprite_sheet->cell_width = cell_width;
  sprite_sheet->cell_height = cell_height;
}
