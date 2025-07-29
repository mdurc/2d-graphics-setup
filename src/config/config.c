#include "config.h"

#include <stdlib.h>

#include "../c-lib/misc.h"
#include "../io/io.h"
#include "../state.h"

#define TMP_BUF_SIZE 20

static const char* CONFIG_DEfAULT =
    "[controls]\n"
    "left = Left\n"
    "right = Right\n"
    "up = Up\n"
    "down = Down\n"
    "escape = Escape\n"
    "debug = F\n";

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

static int key_from_name(const char* name) {
  if (strcmp(name, "Left") == 0) return GLFW_KEY_LEFT;
  if (strcmp(name, "Right") == 0) return GLFW_KEY_RIGHT;
  if (strcmp(name, "Up") == 0) return GLFW_KEY_UP;
  if (strcmp(name, "Down") == 0) return GLFW_KEY_DOWN;
  if (strcmp(name, "Escape") == 0) return GLFW_KEY_ESCAPE;
  if (strcmp(name, "F") == 0) return GLFW_KEY_F;
  return -1;
}

static void config_load_controls(const char* conf_buf) {
  config_key_bind(INPUT_KEY_LEFT, config_get_value(conf_buf, "left"));
  config_key_bind(INPUT_KEY_RIGHT, config_get_value(conf_buf, "right"));
  config_key_bind(INPUT_KEY_UP, config_get_value(conf_buf, "up"));
  config_key_bind(INPUT_KEY_DOWN, config_get_value(conf_buf, "down"));
  config_key_bind(INPUT_KEY_ESCAPE, config_get_value(conf_buf, "escape"));
  config_key_bind(INPUT_KEY_DEBUG, config_get_value(conf_buf, "debug"));
}

static int config_load(void) {
  file_t config_file = io_file_read("./config.ini");
  if (!config_file.is_valid) {
    return 1;
  }
  config_load_controls(config_file.data);
  free(config_file.data);
  return 0;
}

void config_init(void) {
  if (config_load() == 0) {
    LOG("Configuration system initialized and loaded");
    return;
  }
  // otherwise write the default config to disk
  io_file_write((void*)CONFIG_DEfAULT, strlen(CONFIG_DEfAULT), "./config.ini");
  ASSERT(config_load() == 0, "default config did not properly load");
  LOG("Wrote and loaded a default config to disk at: ./config.ini");
}

void config_key_bind(input_key_t key, const char* key_name) {
  int key_code = key_from_name(key_name);
  if (key_code == -1) {
    ERROR_RETURN(, "invalid key name when binding key: %s\n", key_name);
  }

  state.config.keybinds[key] = key_code;
}
