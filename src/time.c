#include "time.h"

#include <SDL2/SDL.h>

#include "state.h"

void time_init(f32 frame_rate) {
  state.time.frame_rate = frame_rate;
  state.time.frame_delay = 1000.f / frame_rate;
}

void time_update(void) {
  state.time.now = (f32)SDL_GetTicks();
  state.time.delta = (state.time.now - state.time.last) / 1000.0f;
  state.time.last = state.time.now;
  ++state.time.frame_count;

  if (state.time.now - state.time.frame_last >= 1000.0f) {
    state.time.frame_rate = state.time.frame_count;
    state.time.frame_count = 0;
    state.time.frame_last = state.time.now;
  }
}

void time_update_late(void) {
  state.time.frame_time = (f32)SDL_GetTicks() - state.time.now;

  if (state.time.frame_delay > state.time.frame_time) {
    SDL_Delay(state.time.frame_delay - state.time.frame_time);
  }
}
