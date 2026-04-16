#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragColor;

// Per-frame uniform
layout(std140, binding = 0) uniform GlobalUBO {
    mat4 view_proj_mat;
    vec3 light_pos; // Light position in world coordinates
    vec3 Kd; // Diffuse coefficient
    vec3 Ld; // Light intensity (light color)
} ubo;

// Per-object uniform
layout(std140, binding = 1) uniform ObjectUBO {
    mat4 u_model_mat;
    mat3 u_normal_mat;
} obj;

// Per-material uniform
layout(std140, binding = 2) uniform MaterialUBO {
    vec3 color;
} material;

// Push constants emulated via uniform (OpenGL doesn't have push constants)
uniform mat4 model_mat;

void main()
{
    // Transform vertex to clip space
    vec4 world_pos = model_mat * vec4(position, 1.0);
    gl_Position = ubo.view_proj_mat * world_pos;

    // Lighting equation
    vec3 t_norm = normalize(obj.u_normal_mat * normal);
    vec3 light_dir = normalize(ubo.light_pos - world_pos.xyz);
    vec3 light_intensity = ubo.Ld * ubo.Kd * max(dot(light_dir, t_norm), 0.0);
    fragColor = material.color * light_intensity;
    //fragColor = material.color;
}
