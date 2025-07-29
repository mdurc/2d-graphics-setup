#ifndef _LIB_MATH_H
#define _LIB_MATH_H

#include "misc.h"
#include "types.h"

#define min(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define max(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#define clamp(_x, _mi, _ma) (max(_mi, min(_x, _ma)))
#define dot(v0, v1) ({ const fv2 _v0 = (v0), _v1 = (v1); (_v0.x * _v1.x) + (_v0.y * _v1.y); })
#define length(v) ({ const fv2 _v = (v); sqrtf(dot(_v, _v)); })
#define normalize(u) ({ const fv2 _u = (u); const f32 _l = length(_u); (fv2) { _u.x / _l, _u.y / _l }; })

#define FLOAT_EPSILON 1e-6f
#define float_eq(a, b) (fabsf((a) - (b)) < FLOAT_EPSILON)

#endif
