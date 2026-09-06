#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 projectionViewMatrix;
} ubo;
layout(set = 1, binding = 0) uniform ObjectUBO {
    float u_test_value;
} obj;
layout(set = 2, binding = 0) uniform MaterialUBO {
    vec3 color;
} material;

layout(push_constant) uniform Push {
    mat4 model_mat;
} push;

void main() {
    gl_Position = ubo.projectionViewMatrix * push.model_mat * vec4(position, 1.0);
    fragTexCoord = texCoord;
    fragColor = material.color;
}
