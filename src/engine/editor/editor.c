#include "editor.h"

#include <ctype.h>
#include <stdlib.h>

#include "../entity/entity.h"
#include "../physics/physics.h"
#include "../state.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

// includes the cimgui C-wrapper, gives access to all the `ig...` functions
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

// includes the cimgui C-wrapper declarations for the backend functions
#include "cimgui_impl.h"

#define LABEL_LEFT(_label) \
  igAlignTextToFramePadding(); \
  igText(_label); \
  igSameLine(0, -1);
#define LABELED_COMBO(_label, _preview, _flags) \
  LABEL_LEFT(_label); \
  if (igBeginCombo("##" _label, _preview, _flags))
#define LABELED_DRAG_FLOAT(_label, _val, _speed, _min, _max) \
    LABEL_LEFT(_label); \
    igDragFloat("##" _label, _val, _speed, _min, _max, "%.2f", 0);
#define LABELED_DRAG_FLOAT2(_label, _val) \
  LABEL_LEFT(_label); \
  igDragFloat2("##" _label, _val, 1.0f, 0, 0, "%.2f", 0);
#define LABELED_INPUT_TEXT(_label, _buf, _size) \
  LABEL_LEFT(_label); \
  igInputText("##" _label, _buf, _size, 0, NULL, NULL);
#define LABELED_CHECKBOX(_label, _val) \
  LABEL_LEFT(_label); \
  igCheckbox("##" _label, _val);
#define LABELED_SLIDER_FLOAT(_label, _val, _min, _max) \
  LABEL_LEFT(_label); \
  igSliderFloat("##" _label, _val, _min, _max, "%.2f", 0);
#define LABELED_INPUT_U8(_label, _val) \
    LABEL_LEFT(_label); \
    igInputScalar("##" _label, ImGuiDataType_U8, _val, NULL, NULL, "%u", 0);
#define OPTION_NONE \
  if (igSelectable_Bool("None", selected_static_body_id == -1, 0, ZERO_VEC))
#define DEFAULT_OPEN ImGuiTreeNodeFlags_DefaultOpen
#define NO_FLAGS 0
#define ZERO_VEC (ImVec2){0, 0}

static ImGuiIO* ioptr; // main ImGui IO object
static int selected_entity_id = -1;
static int selected_body_id = -1;
static int selected_static_body_id = -1;
static bool is_debug_mode = true;
static bool is_visible = false;

// -- new entity/body creation state --
static char new_entity_name[64] = "New Entity";
static vec2 new_entity_pos = {0};
static vec2 new_entity_size = {32, 32};
static bool new_entity_is_kinematic = false;
static vec2 new_static_body_pos = {0};
static vec2 new_static_body_size = {100, 20};

void editor_init(GLFWwindow* window) {
#if __APPLE__
  // GL 3.2 Core + GLSL 150
  const char* glsl_version = "#version 150";
#else
  // GL 3.2 + GLSL 130
  const char* glsl_version = "#version 130";
#endif

  // setup imgui
  igCreateContext(NULL);

  // configuration
  ioptr = igGetIO();
  // ioptr->IniFilename = NULL; // disables the imgui.ini file
  ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#ifdef IMGUI_HAS_DOCK
  ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ioptr->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

  // style and scaling
  ImGuiStyle* style = igGetStyle();
  float x = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

  // there isn't a dynamic style scaling, so this has to be constant.
  // to update, it requires resetting the style and calling this again.
  ImGuiStyle_ScaleAllSizes(style, x);

  // set initial font scale, (using io.ConfigDpiScaleFonts=true makes this
  // unecessary, but is just for completeness)
  style->FontScaleDpi = x;

#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
  // experimental, but should automatically overwrite style.FontScaleDpi in
  // Begin() when the monitor DPI changes. It scales fonts but not
  // sizes/padding.
  ioptr->ConfigDpiScaleFonts = true;

  // experimental, scale windows when monitor DPI changes
  ioptr->ConfigDpiScaleViewports = true;
#endif

  // initialization
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  igStyleColorsDark(NULL);
}

void editor_destroy(void) {
  // clean up
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(NULL);
}

static void imgui_new_frame(void) {
  // start imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();
}

static void render_performance_window(void) {
  if (igCollapsingHeader_TreeNodeFlags("Performance", 0)) {
    igText("FPS: %.1f", ioptr->Framerate);
    igText("Frame Time: %.3f ms", 1000.0f / ioptr->Framerate);
    igText("Delta Time: %.4f s", state.time.delta);
  }
}

static void render_entity_inspector(void) {
  if (igCollapsingHeader_TreeNodeFlags("Entity Inspector", NO_FLAGS)) {
    // -- entity selection --
    const char* preview = (selected_entity_id == -1)
                              ? "None"
                              : entity_get_name(selected_entity_id);

    LABELED_COMBO("Selected Entity", preview, 0) {
      OPTION_NONE { selected_entity_id = -1; }
      for (size_t i = 0; i < entity_count(); i++) {
        entity_t* entity = entity_get(i);
        if (entity->is_active) {
          const bool is_selected = (selected_entity_id == (int)i);
          if (igSelectable_Bool(entity->name, is_selected, 0, ZERO_VEC)) {
            selected_entity_id = i;
          }
          if (is_selected) {
            igSetItemDefaultFocus();
          }
        }
      }
      igEndCombo();
    }

    igSeparator();

    // -- entity properties --
    if (selected_entity_id != -1) {
      entity_t* entity = entity_get(selected_entity_id);
      body_t* body = physics_body_get(entity->body_id);

      igText("Entity ID: %d", selected_entity_id);
      igText("Body ID: %zu", entity->body_id);

      LABELED_DRAG_FLOAT2("Position##ForEntity", body->aabb.position);
      LABELED_DRAG_FLOAT2("Velocity##ForEntity", body->velocity);
      LABELED_CHECKBOX("Is Kinematic##ForEntity", &body->is_kinematic);
      LABELED_DRAG_FLOAT("Gravity Scale##ForEntity", &body->gravity_scale,
                         0.05f, 0.0f, 10.0f);

      if (igButton("Delete Entity", (ImVec2){-1, 0})) {
        entity_deactivate(selected_entity_id);
        selected_entity_id = -1;
      }
    }
  }
}

static const char* get_entity_name_for_body(size_t body_id) {
  for (size_t i = 0; i < entity_count(); ++i) {
    entity_t* entity = entity_get(i);
    if (entity->is_active && entity->body_id == body_id) {
      return entity->name;
    }
  }
  return "Body (No Entity)";
}

static void render_physics_inspector(void) {
  if (igCollapsingHeader_TreeNodeFlags("Physics Inspector", NO_FLAGS)) {
    LABELED_SLIDER_FLOAT("Gravity", physics_get_gravity(), -2000.f, 0.f);
    LABELED_SLIDER_FLOAT("Terminal", physics_get_terminal_velocity(), -7000.f,
                         0.f);
    igText("Body Count: %zu", physics_body_count());
    igText("Static Body Count: %zu", physics_static_body_count());

    igSeparator();

    // -- dynamic body selection --
    const char* body_preview = (selected_body_id == -1)
                                   ? "None"
                                   : get_entity_name_for_body(selected_body_id);
    LABELED_COMBO("Selected Body       ", body_preview, NO_FLAGS) {
      OPTION_NONE { selected_entity_id = -1; }
      for (size_t i = 0; i < physics_body_count(); ++i) {
        body_t* body = physics_body_get(i);
        if (body->is_active) {
          const char* name = get_entity_name_for_body(i);
          if (igSelectable_Bool(name, selected_body_id == (int)i, 0,
                                (ImVec2){0, 0})) {
            selected_body_id = i;
          }
        }
      }
      igEndCombo();
    }

    // -- static body selection --
    char static_body_label[32];
    snprintf(static_body_label, sizeof(static_body_label), "Static Body %d",
             selected_static_body_id);
    const char* static_preview =
        selected_static_body_id == -1 ? "None" : static_body_label;
    LABELED_COMBO("Selected Static Body", static_preview, NO_FLAGS) {
      OPTION_NONE { selected_entity_id = -1; }
      for (size_t i = 0; i < physics_static_body_count(); ++i) {
        snprintf(static_body_label, sizeof(static_body_label),
                 "Static Body %zu", i);
        if (igSelectable_Bool(static_body_label,
                              selected_static_body_id == (int)i, 0,
                              (ImVec2){0, 0})) {
          selected_static_body_id = i;
        }
      }
      igEndCombo();
    }
  }

  // -- selected body properties --
  if (selected_body_id != -1) {
    igSeparator();
    igText("Body Properties");
    body_t* body = physics_body_get(selected_body_id);
    LABELED_DRAG_FLOAT2("Position##ForBody", body->aabb.position);
    LABELED_DRAG_FLOAT2("Size##ForBody", body->aabb.half_size);
    LABELED_DRAG_FLOAT2("Velocity##ForBody", body->velocity);
    LABELED_INPUT_U8("Collision Layer##ForBody", &body->collision_layer);
    LABELED_INPUT_U8("Collision Mask##ForBody", &body->collision_mask);
    LABELED_DRAG_FLOAT("Gravity Scale##ForBody", &body->gravity_scale, 0.05f,
                       0.f, 10.f);
    LABELED_CHECKBOX("Is Kinematic##ForBody", &body->is_kinematic);
  }

  // -- selected static body properties --
  if (selected_static_body_id != -1) {
    igSeparator();
    igText("Static Body Properties");
    static_body_t* static_body =
        physics_static_body_get(selected_static_body_id);
    LABELED_DRAG_FLOAT2("Position##ForStaticBody", static_body->aabb.position);
    LABELED_DRAG_FLOAT2("Half Size##ForStaticBody",
                        static_body->aabb.half_size);
    LABELED_INPUT_U8("Collision Layer##ForStaticBody",
                     &static_body->collision_layer);
  }
}

static void increment_string_number(char* str, size_t size) {
  size_t len = strlen(str);
  char* end = str + len;
  char* p = end;
  while (p > str && isdigit(*(p - 1))) {
    p--;
  }
  if (p == end) {
    // no number found at the end
    snprintf(end, size - len, " 1");
  } else {
    long num = strtol(p, NULL, 10);
    snprintf(p, size - (p - str), "%ld", num + 1);
  }
}

static void render_creation_tools(void) {
  if (igCollapsingHeader_TreeNodeFlags("Creation Tools", NO_FLAGS)) {
    // -- create entity --
    LABELED_INPUT_TEXT("Name##ForCreation", new_entity_name,
                       sizeof(new_entity_name));
    LABELED_DRAG_FLOAT2("Position##ForCreation", new_entity_pos);
    LABELED_DRAG_FLOAT2("Size##ForCreation", new_entity_size);
    LABELED_CHECKBOX("Is Kinematic##ForCreation", &new_entity_is_kinematic);
    if (igButton("Create Entity", (ImVec2){-1, 0})) {
      entity_create(new_entity_name, new_entity_pos, new_entity_size,
                    (vec2){0, 0}, 1.0f, 0, 0, new_entity_is_kinematic,
                    (size_t)-1, NULL, NULL);
      increment_string_number(new_entity_name, sizeof(new_entity_name));
    }

    igSeparator();

    // -- create static body --
    igText("Create Static Body");
    LABELED_DRAG_FLOAT2("Position Static##ForStaticCreation",
                        new_static_body_pos);
    LABELED_DRAG_FLOAT2("Size Static##ForStaticCreation", new_static_body_size);
    if (igButton("Create Static Body", (ImVec2){-1, 0})) {
      physics_static_body_create(new_static_body_pos, new_static_body_size, 0);
    }
  }
}

void editor_update(void) {
  imgui_new_frame();

  if (!is_visible) {
    return;
  }

  igSetNextWindowSize((ImVec2){600, 600}, ImGuiCond_Once);
  igBegin("Engine Editor", &is_visible, 0);

  LABELED_CHECKBOX("Debug Mode##Global", &is_debug_mode);
  igSeparator();

  render_performance_window();
  render_entity_inspector();
  render_physics_inspector();
  render_creation_tools();

  igEnd();
}

void editor_render(void) {
  // render
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
#ifdef IMGUI_HAS_DOCK
  if (ioptr->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_window = glfwGetCurrentContext();
    igUpdatePlatformWindows();
    igRenderPlatformWindowsDefault(NULL, NULL);
    glfwMakeContextCurrent(backup_current_window);
  }
#endif
}

bool editor_is_debug(void) { return is_debug_mode; }
bool editor_is_visible(void) { return is_visible; }
void editor_toggle_visibility(void) { is_visible = !is_visible; }
