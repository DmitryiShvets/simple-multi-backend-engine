struct GlobalUBO {
  float4x4 view_projection;
  float3 light_position;
  float shadow_depth_scale;
  float3 Kd;
  float shadow_uv_scale;
  float3 Ld;
  float3 camera_position;
  float4x4 light_view_projection;
};

struct Push {
  float4x4 model_mat;
};

ConstantBuffer<GlobalUBO> g_ubo : register(b0, space0);
[[vk::push_constant]] ConstantBuffer<Push> push : register(b0, space3);

struct VSOutput {
  float4 position : SV_POSITION;
};

[shader("vertex")]
VSOutput main(float3 position : POSITION0) {
  VSOutput o;
  float4 world_pos = mul(push.model_mat, float4(position, 1.0));
  o.position = mul(g_ubo.light_view_projection, world_pos);
  return o;
}
