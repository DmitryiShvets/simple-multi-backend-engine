#version 460 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragColor;

layout(std140, binding = 0) uniform GlobalUBO { mat4 projectionViewMatrix; } ubo;
layout(std140, binding = 1) uniform ObjectUBO { float u_test_value; } obj;
layout(std140, binding = 2) uniform MaterialUBO { vec3 color; } material;

uniform mat4 model_mat;

void main() {
    gl_Position = ubo.projectionViewMatrix * model_mat * vec4(position, 1.0);
    fragTexCoord = texCoord;
    fragColor = material.color;
}
