#version 460 core

// Include common block
#include "global.inc"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragColor;

// Per-object uniform
layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 u_model_mat;
    mat3 u_normal_mat;
} obj;

// Per-material uniform
layout(set = 2, binding = 0) uniform MaterialUBO {
    vec3 color;
} material;

// Push constants for model matrix
layout(push_constant) uniform Push {
    mat4 model_mat;
} push;

void main()
{
    // Transform vertex to clip space
    vec4 world_pos = push.model_mat * vec4(position, 1.0);
    //vec4 world_pos = vec4(0.0,0.0,0.0,1.0);
    gl_Position = ubo.view_proj_mat * world_pos;

    // Lighting equation
    vec3 t_norm = normalize(obj.u_normal_mat * normal);
    vec3 light_dir = normalize(ubo.light_pos - world_pos.xyz);
    vec3 light_intensity = ubo.Kd * ubo.Kd * max(dot(light_dir, t_norm), 0.0);
    //fragColor =  light_intensity;
    //fragColor =  light_dir;
    //fragColor =  ubo.Ld;
    //fragColor =  ubo.Kd;
    //fragColor =  ubo.light_pos;
    fragColor = material.color * light_intensity;
    //fragColor = material.color;
    //fragColor = t_norm * 0.5 + 0.5;
    //fragColor = normal * 0.5 + 0.5;
    //fragColor = vec3(1.0,1.0,1.0) * dot(light_dir, t_norm);
    //  fragColor = vec3(1.0,1.0,1.0) * max(dot(light_dir, t_norm), 0.0);
    // fragColor = light_intensity;
}
