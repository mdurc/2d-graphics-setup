#pragma once

#include <stdbool.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// compatibility macro for older versions that had an experimental string type
#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif

// another compatibility macro: a function declared with () is different from
// one with (void) in C, and this makes sure it links to the correct one
#define igGetIO igGetIO_Nil

void editor_init(GLFWwindow* window);
void editor_destroy(void);

void editor_update(void);
void editor_render(void);

bool editor_is_debug(void);
void editor_toggle_visibility(void);
