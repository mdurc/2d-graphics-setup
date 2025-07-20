#ifndef _LIB_MISC_H
#define _LIB_MISC_H

#include <stdio.h>
#include <stdlib.h> // for exit(1);

#include "log.h"
#include "macros.h"

#define _ASSERTX(_e, _expr, _fmt, ...)      \
  do {                                      \
    if (!(_e)) {                            \
      ERROR("(assertion expr %s)", _expr);  \
      ERROR((_fmt), ##__VA_ARGS__);         \
      exit(1);                              \
    }                                       \
  } while (0)
#define _ASSERT1(_e) _ASSERTX(_e, #_e, "%s", "assertion failed")
#define _ASSERT2(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT3(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT4(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT5(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT6(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT7(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define _ASSERT8(_e, ...) _ASSERTX(_e, #_e, __VA_ARGS__)
#define ASSERT(...) DISPATCH_N(_ASSERT, __VA_ARGS__)

// #define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

#define ARRLEN(_arr) ((sizeof((_arr))) / ((sizeof((_arr)[0]))))
#define R_MASK 0x000000FF
#define G_MASK 0x0000FF00
#define B_MASK 0x00FF0000
#define A_MASK 0xFF000000

#endif
