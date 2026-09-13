struct GlobalUBO {
  float4x4 view_projection;
  float3 light_position;
  float3 Kd;
  float3 Ld;
  float3 camera_position;
};
struct ObjectUBO {
  float4x4 u_model_mat;
  float3x3 u_normal_mat;
};
struct MaterialUBO { float3 color; };

[[vk::binding(0, 0)]] ConstantBuffer<GlobalUBO> g_ubo;
[[vk::binding(0, 1)]] ConstantBuffer<ObjectUBO> o_ubo;
[[vk::binding(0, 2)]] ConstantBuffer<MaterialUBO> m_ubo;

struct Push { float4x4 model_mat; };
[[vk::push_constant]] ConstantBuffer<Push> model_mat;

struct VSOutput {
  float4 position : SV_POSITION;
  float3 color : COLOR0;
};

[shader("vertex")]
VSOutput main(float3 position : POSITION0,
              float3 normal : NORMAL0,
              float2 texCoord : TEXCOORD0) {
  VSOutput o;
  float4 world_pos = mul(model_mat.model_mat, float4(position, 1.0));
  o.position = mul(g_ubo.view_projection, world_pos);
  float3 t_norm = normalize(mul(o_ubo.u_normal_mat, normal));
  float3 light_dir = normalize(g_ubo.light_position - world_pos.xyz);
  float3 light_intensity = g_ubo.Kd * g_ubo.Ld * max(dot(light_dir, t_norm), 0.0);
  o.color = m_ubo.color * light_intensity;
  return o;
}
