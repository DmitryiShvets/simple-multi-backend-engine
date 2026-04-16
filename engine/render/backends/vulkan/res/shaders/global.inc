// GlobalUBO header

#ifndef GLOBAL_INC
#define GLOBAL_INC

// Per-frame uniform
layout(set = 0, binding = 0) uniform GlobalUBO {
  mat4 view_proj_mat;
  vec3 light_pos;   // Light position in view coordinates
  vec3 Kd;          // Diffuse coefficient
  vec3 Ld;          // Light intensity
} ubo;

#endif
