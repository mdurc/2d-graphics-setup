#ifndef _LIB_TIME_H
#define _LIB_TIME_H

#include <SDL2/SDL.h>

#include "macros.h"
#include "types.h"

#define ns_to_sec(_ns) ((_ns) / 1000000000.0)
#define sec_to_ns(_s) ((_s) * 1000000000.0)

static u64 time_start = 0, time_freq = 0;

M_INLINE u64 time_ns() {
  const u64 now = SDL_GetPerformanceCounter();

  if (!time_start) {
    time_start = now;
  }

  if (!time_freq) {
    time_freq = SDL_GetPerformanceFrequency();
  }

  const u64 diff = now - time_start;
  return (diff / (f64)time_freq) * 1000000000.0;
}

M_INLINE f64 time_s() { return time_ns() / 1000000000.0; }

#endif
