#include "render_init.h"

#include "../c-lib/misc.h"
#include "../io/io.h"
#include "../state.h"

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

  printf("OpenGL Loaded\n");
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));

  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum nr of vertex attributes supported: %d\n", nrAttributes);
}

u32 render_create_shader(const char* path_vert, const char* path_frag) {
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

void render_init_quad(u32* vao, u32* vbo, u32* ebo) {
  // when drawing two triangles, we will have 2 extra vertices, which is not ok.
  // EBO, element buffer objects select a specific order of vertices to use.

  // only 4 vertices here instead of 6
  f32 vertices[] = {
      // position         // color
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, // top left
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
  glEnableVertexAttribArray(0);

  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the vbo, we linked it with attrib

  // do NOT unbind the EBO while a VAO is active as the bound element
  // buffer object IS stored in the VAO; keep the EBO bound.

  glBindVertexArray(0); // unbind the vao

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // for wireframe mode
}
