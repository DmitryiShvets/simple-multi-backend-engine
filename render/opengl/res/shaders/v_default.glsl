#version 460 core

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

// Per-frame uniform
layout(std140, binding = 0) uniform GlobalUBO {
  mat4 projectionViewMatrix;
} ubo;

// Per-material uniform
layout(std140, binding = 1) uniform MaterialUBO {
  vec3 color;
} material;

// Push constants эмулируются через uniform (OpenGL не имеет push constants)
uniform mat4 modelMatrix;

void main() {
  gl_Position = ubo.projectionViewMatrix * modelMatrix * vec4(position, 1.0);
  fragColor = material.color;
}
