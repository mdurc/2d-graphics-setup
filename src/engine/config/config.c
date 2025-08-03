#include "config.h"

#include <stdlib.h>

#include "../c-lib/misc.h"
#include "../io/io.h"
#include "../state.h"

static const keymap_t glfw_keymap[] = {
    {"Left", GLFW_KEY_LEFT}, {"Right", GLFW_KEY_RIGHT},   {"Up", GLFW_KEY_UP},
    {"Down", GLFW_KEY_DOWN}, {"Escape", GLFW_KEY_ESCAPE}, {"F", GLFW_KEY_F},
    {"O", GLFW_KEY_O},       {"Space", GLFW_KEY_SPACE},   {"W", GLFW_KEY_W},
    {"A", GLFW_KEY_A},       {"S", GLFW_KEY_S},           {"D", GLFW_KEY_D},
};
static const keybind_info_t config_info[] = {
    {INPUT_KEY_LEFT, "left", "Left"},
    {INPUT_KEY_RIGHT, "right", "Right"},
    {INPUT_KEY_UP, "up", "Up"},
    {INPUT_KEY_DOWN, "down", "Down"},
    {INPUT_KEY_ESCAPE, "escape", "Escape"},
    {INPUT_KEY_DEBUG, "debug", "F"},
    {INPUT_KEY_EDITOR_TOGGLE, "editor", "O"},
};
static const size_t glfw_keymap_size = sizeof(glfw_keymap) / sizeof(keymap_t);
static const size_t config_size = sizeof(config_info) / sizeof(keybind_info_t);

#define TMP_BUF_SIZE 20
static char tmp_buf[TMP_BUF_SIZE] = {0};

static char* config_get_value(const char* conf_buf, const char* value) {
  char* line = strstr(conf_buf, value);
  if (!line) {
    ERROR_EXIT("couldn't find the config value: %s\n", value);
  }
  size_t len = strlen(line);
  char *curr = line, *end = line + len;
  char* tmp_ptr = &tmp_buf[0];

  while (*curr != '=' && curr != end) {
    ++curr;
  }
  ++curr; // skip the '='
  while (*curr == ' ') {
    ++curr;
  }
  while (*curr != '\n' && curr != end && (tmp_ptr - tmp_buf) < TMP_BUF_SIZE) {
    *tmp_ptr++ = *curr++;
  }
  *tmp_ptr = '\0';

  return tmp_buf;
}

static i32 key_from_name(const char* name) {
  for (size_t i = 0; i < glfw_keymap_size; ++i) {
    if (strcmp(name, glfw_keymap[i].name) == 0) {
      return glfw_keymap[i].glfw_key_code;
    }
  }
  return -1;
}

static void config_load_controls(const char* conf_buf) {
  for (size_t i = 0; i < config_size; ++i) {
    const keybind_info_t* info = &config_info[i];
    const char* key_name = config_get_value(conf_buf, info->name_in_config);
    config_key_bind(info->key, key_name);
  }
}

static i32 config_load(void) {
  file_t config_file = io_file_read("./config.ini");
  if (!config_file.is_valid) {
    return -1;
  }
  config_load_controls(config_file.data);
  free(config_file.data);
  return 0;
}

static void write_default_config(void) {
  char buffer[1024] = "[controls]\n";
  for (size_t i = 0; i < config_size; ++i) {
    const keybind_info_t* info = &config_info[i];
    char line[128];
    snprintf(line, sizeof(line), "%s = %s\n", info->name_in_config,
             info->default_key);
    strcat(buffer, line);
  }
  io_file_write(buffer, strlen(buffer), "./config.ini");
  LOG("Wrote and loaded a default config to disk at: ./config.ini");
}

void config_init(void) {
  if (config_load() == 0) {
    LOG("Configuration system initialized and loaded");
    return;
  }

  write_default_config();
  LOG("wrote and loaded a default config to disk at: ./config.ini");
  if (config_load() != 0) {
    ERROR_EXIT("default config did not properly load.");
  }
}

void config_key_bind(input_key_t key, const char* key_name) {
  i32 key_code = key_from_name(key_name);
  if (key_code == -1) {
    ERROR_EXIT("invalid key name when binding key: %s\n", key_name);
  }
  ASSERT(key_code >= 0);
  state.config.keybinds[key] = key_code;
}
