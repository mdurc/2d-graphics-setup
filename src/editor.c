#include "editor.h"

#include "entity/entity.h"
#include "game/data.h"
#include "state.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

// includes the cimgui C-wrapper, gives access to all the `ig...` functions
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

// includes the cimgui C-wrapper declarations for the backend functions
#include "cimgui_impl.h"

static ImGuiIO* ioptr; // main ImGui IO object
static int selected_entity_id = -1;
static bool is_debug_mode = true;
static bool editor_is_visible = false;

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
  ioptr->IniFilename = NULL; // disables the imgui.ini file
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

void editor_update(void) {
  imgui_new_frame();

  if (!editor_is_visible) {
    return;
  }

  igSetNextWindowSize((ImVec2){400, 400}, ImGuiCond_Once);

  igBegin("Editor", NULL, 0);

  igCheckbox("Debug Mode", &is_debug_mode);

  // -- performance --
  if (igCollapsingHeader_TreeNodeFlags("Performance",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    igText("FPS: %.1f", ioptr->Framerate);
    igText("Frame Time: %.3f ms", 1000.0f / ioptr->Framerate);
    igText("Delta Time: %.4f s", state.time.delta);
  }

  // -- Physics Controls --
  if (igCollapsingHeader_TreeNodeFlags("Physics",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    // Pause/Play button
    if (igButton(is_paused ? "Play" : "Pause", (ImVec2){0, 0})) {
      is_paused = !is_paused;
    }

    // Advance one frame if paused
    if (is_paused) {
      igSameLine(0, 5);
      if (igButton("Step Frame", (ImVec2){0, 0})) {
        advance_frame = true;
      }
    }

    igSliderFloat("Gravity", physics_get_gravity(), -2000.f, 0.f, "%.2f", 0);
  }

  // -- Entity Inspector --
  if (igCollapsingHeader_TreeNodeFlags("Entity Inspector",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    const char* preview = (selected_entity_id == -1)
                              ? "None"
                              : entity_get_name(selected_entity_id);
    if (igBeginCombo("Selected Entity", preview, 0)) {
      for (size_t i = 0; i < entity_count(); i++) {
        if (entity_get(i)->is_active) {
          const bool is_selected = (selected_entity_id == (int)i);
          if (igSelectable_Bool(entity_get_name(i), is_selected, 0,
                                (ImVec2){0, 0})) {
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

    if (selected_entity_id != -1) {
      entity_t* entity = entity_get(selected_entity_id);
      body_t* body = physics_body_get(entity->body_id);

      igText("Entity ID: %d", selected_entity_id);
      igText("Body ID: %zu", entity->body_id);
      igText("Animation ID: %zu", entity->animation_id);

      igSeparator();

      igDragFloat2("Position", body->aabb.position, 1.0f, 0, 0, "%.2f", 0);
      igDragFloat2("Velocity", body->velocity, 1.0f, 0, 0, "%.2f", 0);
      igCheckbox("Is Kinematic", &body->is_kinematic);
    }
  }

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
void editor_toggle_visibility(void) { editor_is_visible = !editor_is_visible; }
