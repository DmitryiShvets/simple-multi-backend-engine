#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragColor;

// Per-frame uniform
layout(std140, binding = 0) uniform GlobalUBO {
  mat4 view_proj_mat;
  vec3 light_pos;   // Позиция источника света в видимых координатах
  vec3 Kd;          // Коэффициент рассеивания
  vec3 Ld;          // Интенсивность источника света (цвет света)
} ubo;

// Per-material uniform
layout(std140, binding = 1) uniform MaterialUBO {
  vec3 color;
} material;

// Per-object uniform
layout(std140, binding = 2) uniform ObjectUBO {
  mat4 normal_mat1;
  mat3 normal_mat;
} obj;

// Push constants эмулируются через uniform (OpenGL не имеет push constants)
uniform mat4 model_mat;

void main()
{
  // Transform vertex to clip space
  vec4 world_pos = model_mat * vec4(position, 1.0);
  gl_Position = ubo.view_proj_mat * world_pos;

  // Lighting equation
  vec3 t_norm = normalize(obj.normal_mat * normal);
  vec3 light_dir = normalize(ubo.light_pos - world_pos.xyz);
  vec3 light_intensity = ubo.Ld * ubo.Kd * max(dot(light_dir, t_norm), 0.0);
  fragColor = material.color * light_intensity;
}
