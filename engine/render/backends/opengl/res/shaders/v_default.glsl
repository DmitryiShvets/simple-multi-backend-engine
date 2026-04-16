#version 460 core

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

// Per-frame uniform
layout(std140, binding = 0) uniform GlobalUBO {
    mat4 projectionViewMatrix;
} ubo;
// Per-object uniform
layout(std140, binding = 1) uniform ObjectUBO {
    float u_test_value;
} obj;
// Per-material uniform
layout(std140, binding = 2) uniform MaterialUBO {
    vec3 color;
} material;

// Push constants emulated via uniform (OpenGL doesn't have push constants)
uniform mat4 model_mat;

void main() {
    gl_Position = ubo.projectionViewMatrix * model_mat * vec4(position, 1.0);
    fragColor = material.color;
}
