#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

// Per-frame uniform buffer (Descriptor Set 0, Binding 0)
layout(set = 0, binding = 0) uniform GlobalUBO {
  mat4 projectionViewMatrix;
} ubo;

// Per-material uniform buffer (Descriptor Set 1, Binding 0)
layout(set = 1, binding = 0) uniform MaterialUBO {
  vec3 color;
} material;

// Push constants для model matrix
layout(push_constant) uniform Push {
  mat4 model_mat;
} push;


void main() {
  gl_Position = ubo.projectionViewMatrix * push.model_mat * vec4(position, 1.0);
  fragColor = material.color;
}
