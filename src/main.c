#include "animation/animation.h"
#include "audio/audio.h"
#include "config/config.h"
#include "entity/entity.h"
#include "game/game.h"
#include "physics/physics.h"
#include "renderer/render.h"
#include "state.h"
#include "time/time.h"

state_t state;

int main(void) {
  time_init(60);
  config_init();
  render_init(1280, 720, 3.0f);
  physics_init();
  entity_init();
  animation_init();
  audio_init();

  run_game_loop();

  audio_destroy();
  animation_destroy();
  entity_destroy();
  physics_destroy();
  render_destroy();
}
