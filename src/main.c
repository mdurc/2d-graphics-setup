#include "background.h"
#include "c-lib/dynlist.h"
#include "c-lib/time.h"
#include "config/config.h"
#include "font.h"
#include "renderer/render.h"
#include "sprite_sheet/img.h"
#include "sprite_sheet/sprite_sheet.h"
#include "state.h"
#include "time.h"

static bool should_quit = false;
static fv2 pos;
static input_key_t last_dir = INPUT_KEY_LEFT;
static int animation_idx = 1;

static void input_handle(void) {
  if (state.input.left == KS_PRESSED || state.input.left == KS_HELD) {
    pos.x -= 500 * state.time.delta;
    last_dir = INPUT_KEY_LEFT;
    animation_idx = 1;
  }
  if (state.input.right == KS_PRESSED || state.input.right == KS_HELD) {
    pos.x += 500 * state.time.delta;
    last_dir = INPUT_KEY_RIGHT;
    animation_idx = 2;
  }
  if (state.input.up == KS_PRESSED || state.input.up == KS_HELD) {
    pos.y -= 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 3 : 4;
  }
  if (state.input.down == KS_PRESSED || state.input.down == KS_HELD) {
    pos.y += 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 5 : 6;
  }
  if (state.input.escape == KS_PRESSED || state.input.escape == KS_HELD) {
    should_quit = true;
  }
}

int main(void) {
  time_init(60);
  config_init();
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT);

  state_init();

  pos = (fv2){SCREEN_WIDTH / 2.0f - 100, SCREEN_HEIGHT / 2.0f - 100};

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  while (!should_quit) {
    time_update();

    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT: should_quit = true; break;
        default: break;
      }
    }

    input_update();
    input_handle();

    sprite_t player = {.src_idx = (iv2){animation_idx, 0},
                       .dst_px = pos,
                       .rotation = 0.0f,
                       .src_sheet = &state.bg_sheet};

    f32 t = 100.0f + (cosf(time_s()) * 100.0f);

    DYNLIST(sprite_t) batch = dynlist_create(sprite_t);

    push_background(&batch, &state.bg_sheet);
    push_font_ch(&batch, &state.font_sheet, 'A', (fv2){t, t});
    push_font_str(&batch, &state.font_sheet, msg,
                  (fv2){t, t + 8.0f * state.font_sheet.scale});

    render_begin();
    render_batch(&batch, true);
    render_sprite(&player, 46.0f);
    render_end();
  }

  state_destroy();
}
