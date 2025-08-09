#include "math.h"

// ---- vector implementations ----
void vec2_scale(vec2 result, const vec2 a, f32 s) {
  result[0] = a[0] * s;
  result[1] = a[1] * s;
}
void vec2_add(vec2 result, const vec2 a, const vec2 b) {
  result[0] = a[0] + b[0];
  result[1] = a[1] + b[1];
}
void vec2_sub(vec2 result, const vec2 a, const vec2 b) {
  result[0] = a[0] - b[0];
  result[1] = a[1] - b[1];
}
void vec3_add(vec3 result, const vec3 a, const vec3 b) {
  result[0] = a[0] + b[0];
  result[1] = a[1] + b[1];
  result[2] = a[2] + b[2];
}
void vec3_sub(vec3 result, const vec3 a, const vec3 b) {
  result[0] = a[0] - b[0];
  result[1] = a[1] - b[1];
  result[2] = a[2] - b[2];
}
f32 vec3_dot(const vec3 a, const vec3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
f32 vec3_len(const vec3 v) { return sqrtf(vec3_dot(v, v)); }
void vec3_cross(vec3 result, const vec3 a, const vec3 b) {
  result[0] = a[1] * b[2] - a[2] * b[1];
  result[1] = a[2] * b[0] - a[0] * b[2];
  result[2] = a[0] * b[1] - a[1] * b[0];
}
void vec3_normalize(vec3 result, const vec3 v) {
  f32 len = vec3_len(v);
  if (len == 0.0f) {
    result[0] = result[1] = result[2] = 0.0f;
    return;
  }
  f32 inv_len = 1.0f / len;
  result[0] = v[0] * inv_len;
  result[1] = v[1] * inv_len;
  result[2] = v[2] * inv_len;
}
void vec3_scale(vec3 result, const vec3 a, f32 s) {
  result[0] = a[0] * s;
  result[1] = a[1] * s;
  result[2] = a[2] * s;
}
void vec4_scale(vec4 result, const vec4 a, f32 s) {
  result[0] = a[0] * s;
  result[1] = a[1] * s;
  result[2] = a[2] * s;
  result[3] = a[3] * s;
}

// ---- matrix implementations ----
void mat4x4_identity(mat4x4* mat) {
  memset(mat->data, 0, sizeof(f32) * 16);
  mat->data[0] = 1.0f;
  mat->data[5] = 1.0f;
  mat->data[10] = 1.0f;
  mat->data[15] = 1.0f;
}
void mat4x4_mul(mat4x4* result, const mat4x4* a, const mat4x4* b) {
  mat4x4 temp;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      temp.data[r * 4 + c] = 0.0f;
      for (int i = 0; i < 4; ++i) {
        temp.data[r * 4 + c] += a->data[r * 4 + i] * b->data[i * 4 + c];
      }
    }
  }
  memcpy(result->data, temp.data, sizeof(f32) * 16);
}
void mat4x4_from_translation(mat4x4* mat, f32 x, f32 y, f32 z) {
  /*
     1.0f, 0.0f, 0.0f, x, // x translation
     0.0f, 1.0f, 0.0f, y, // y translation
     0.0f, 0.0f, 1.0f, z, // z translation
     0.0f, 0.0f, 0.0f, 1.0f,
  */
  mat4x4_identity(mat);
  mat->data[3] = x;
  mat->data[7] = y;
  mat->data[11] = z;
}
void mat4x4_translate(mat4x4* mat, f32 x, f32 y, f32 z) {
  mat4x4 t;
  mat4x4_from_translation(&t, x, y, z);
  mat4x4_mul(mat, mat, &t);
}

/*
   Note that the outermost row/col is for the translation, so we really only
   need a 3x3 for 3d rotation, though the extra trick is useful.

   Solving for the 2d rotation matrix:
   in polar, we know that x = r cos(t) and y = r sin(t)
   thus, for some arbitrary rotation p: x' = r cos(t + p), y' = r sin(t + p)

   Using trig identities, we can rewrite x' and y' as:

   x' = r(cos(p)cos(t)) - r(sin(p)sin(t))
   y' = r(sin(p)cos(t)) + r(cos(p)sin(t))

   thus, substituting in x or y for their respective polar coordinate values:

   x' = xcos(p) - ysin(p)
   y' = xsin(p) + ycos(p)

   thus, we know that the matrix to multiply by the position vector is:
   |cos -sin| times |x|
   |sin  cos|       |y|

   For 3d, to rotate around the z-axis, it is the same as this 2d rotation,
   and just have to make it so that z is not affected.
*/
void mat4x4_rotate_z(mat4x4* mat, f32 theta) {
  f32 s = sinf(theta);
  f32 c = cosf(theta);
  mat4x4 r = {
      c,    -s,   0.0f, 0.0f, // x' = cx - sy
      s,    c,    0.0f, 0.0f, // y' = sx + cy
      0.0f, 0.0f, 1.0f, 0.0f, // z' = z
      0.0f, 0.0f, 0.0f, 1.0f,
  };
  mat4x4_mul(mat, mat, &r);
}

/*
   For rotation around the x and y axis, we can imagine a simple rotation of the
   entire coordinate plane and what the corresponding values for 'x' and 'y'
   would be in the formula for the 2d rotation matrix.

   For example, in the rotation around the x-axis:
   imagine that x is the vertical (old z) axis. If this were to happen, then
   the y axis must have been rotated to become the old x-axis, and the z axis
   would have been rotated to become the old y-axis.

   after rotating the entire 3d coordinate plane, we can note that now:
   z = rsin(t) and y = rcos(t) and following the formula from before:
   y' = cos(p)y - sin(p)z
   z' = sin(p)y + cos(p)z
*/
void mat4x4_rotate_x(mat4x4* mat, f32 theta) {
  f32 s = sinf(theta);
  f32 c = cosf(theta);
  mat4x4 r = {
      1.0f, 0.0f, 0.0f, 0.0f, // x' = x
      0.0f, c,    -s,   0.0f, // y' = cy - sz
      0.0f, s,    c,    0.0f, // z' = sy + cz
      0.0f, 0.0f, 0.0f, 1.0f,
  };
  mat4x4_mul(mat, mat, &r);
}

/*
   Same as with the x-axis rotation, we can imagine that the entire 3d
   coordinate plane is rotated to have the y-axis pointing upwards. Then the new
   polar coordinates would be: x = rsin(t) and z = rcos(t).
   Thus:
   x = sin(p)z + cos(p)x
   z = cos(p)z - sin(p)x
*/
void mat4x4_rotate_y(mat4x4* mat, f32 theta) {
  f32 s = sinf(theta);
  f32 c = cosf(theta);
  mat4x4 r = {
      c,    0.0f, s,    0.0f, // x' = cx + sz
      0.0f, 1.0f, 0.0f, 0.0f, // y' = y
      -s,   0.0f, c,    0.0f, // z' = -sx + cz
      0.0f, 0.0f, 0.0f, 1.0f,
  };
  mat4x4_mul(mat, mat, &r);
}

/*
   Opengl will automatically divide the homogeneous component W on the gpu.
   Thus we can set W to Z to scale Z for depth.
   Note that in opengl:
   - Right-handed view space: camera is at (0,0,0) looking down the -Z axis
   - Left-handed NDC space: [-1, 1] cube where the +Z is behind the cube
   It is possible to make it so that the NDC space is also right handed,
   though that is not the opengl convention/standard. Because of this, we can
   set W to be negative for proper scaling.

   | c 0 0  0 | times |x| equals |xc|
   | 0 d 0  0 |       |y|        |yd|
   | 0 0 a  b |       |z|        |za + b|
   | 0 0 -1 0 |       |1|        |-z|

   We can then solve for za + b to make sure that it is bound in the NDC range
   of [-1, 1]. We can then plug in the two extreme distances in for z, note
   that they will be -n and -f, and negative because they are not behind the
   screen (LHS opengl standard). Thus we have two constraints to solve a
   system of equations:

   z_ndc = z_clip / w_clip = (az + b) / -(z)

   so:
   (a(-n) + b) / -(-n) = -1
   (a(-f) + b) / -(-f) =  1

   b = -n + an
   -af - n + an = f
   a(-f + n) = f + n

   a = (f + n) / (n - f)
   b = -n + (fn + n^2) / (n - f) = 2fn / (n - f)

   For a simple orthographic projection, just leave c and d as 1.0f.
   We already have scaling now proportional to the Z component for depth. We
   can further customize our projection based on FOV and aspect ratio (y and x
   scaling). Just like with z:
   x_ndc = x_clip / w_clip = (xc) / -(z)
   y_ndc = y_clip / w_clip = (yd) / -(z)

   If you image a side-view of the camera to the near facing plane on the
   screen, the cone can be split down the middle to create two
   right-triangles. The top right vertex is y_top, and the bottom right vertex
   is y_bot, and the FOV is the full angle theta. We know that the height:
   y_top = n * tan(FOV/2).
   - We can generalize this by noting that this relationship is constant
   between any top edge, thus y = -z * tan(FOV/2).

   To solve for d, the y-scaling, plug it into y_ndc:
   y_ndc = (-z * tan(FOV/2)) d / -z = tan(FOV/2) * d,
   thus d = y_ndc / tan(FOV/2),
   thus, since we want to map y_ndc to 1 at this top edge, let y_ndc = 1:
   then, d = 1 / tan(FOV/2)

   To solve for c, the x-scaling, use the aspect ratio (width / height) to
   counteract any stretching: c = d / aspect_ratio
*/
void mat4x4_perspective(mat4x4* mat, f32 fov_radians, f32 aspect, f32 n,
                        f32 f) {
  // fov = pi / 2.0f for an orthographic projection
  f32 d = 1.0f / tanf(fov_radians / 2.0f);
  f32 c = d / aspect;
  f32 a = (f + n) / (n - f);
  f32 b = (2.0f * f * n) / (n - f);

  *mat = (mat4x4){
      c,    0.0f, 0.0f,  0.0f, // x' = x
      0.0f, d,    0.0f,  0.0f, // y' = y
      0.0f, 0.0f, a,     b,    // z' = za + b
      0.0f, 0.0f, -1.0f, 0.0f, // w' = -z
  };
}

// non-uniform scale to a mtrix
void mat4x4_scale_aniso(mat4x4* mat, f32 sx, f32 sy, f32 sz) {
  mat4x4 s = {
      sx,   0.0f, 0.0f, 0.0f, // x' = x * x
      0.0f, sy,   0.0f, 0.0f, // y' = y * y
      0.0f, 0.0f, sz,   0.0f, // z' = z * z
      0.0f, 0.0f, 0.0f, 1.0f,
  };
  mat4x4_mul(mat, mat, &s);
}

// orthographic projection maps the 3d volume to the 2d screen without any
// perspective distortion.
void mat4x4_ortho(mat4x4* mat, f32 left, f32 right, f32 bottom, f32 top,
                  f32 near, f32 far) {
  mat4x4_identity(mat);
  mat->data[0] = 2.0f / (right - left);
  mat->data[5] = 2.0f / (top - bottom);
  mat->data[10] = -2.0f / (far - near);
  mat->data[3] = -(right + left) / (right - left);
  mat->data[7] = -(top + bottom) / (top - bottom);
  mat->data[11] = -(far + near) / (far - near);
}

void mat4x4_look_at(mat4x4* mat, const vec3 eye, const vec3 center,
                    const vec3 up) {
  vec3 f, s, u;

  // z-axis (forward)
  vec3_sub(f, center, eye);
  vec3_normalize(f, f);

  // x-axis (side)
  vec3_cross(s, f, up);
  vec3_normalize(s, s);

  // y-axis (up)
  vec3_cross(u, s, f);

  mat->data[0] = s[0];
  mat->data[1] = s[1];
  mat->data[2] = s[2];
  mat->data[3] = -vec3_dot(s, eye);

  mat->data[4] = u[0];
  mat->data[5] = u[1];
  mat->data[6] = u[2];
  mat->data[7] = -vec3_dot(u, eye);

  mat->data[8] = -f[0];
  mat->data[9] = -f[1];
  mat->data[10] = -f[2];
  mat->data[11] = vec3_dot(f, eye);

  mat->data[12] = 0.0f;
  mat->data[13] = 0.0f;
  mat->data[14] = 0.0f;
  mat->data[15] = 1.0f;
}
