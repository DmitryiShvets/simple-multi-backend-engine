struct GlobalUBO {
  float4x4 view_projection;
  float3 light_position;
  float3 Kd;
  float3 Ld;
  float3 camera_position;
};
struct ObjectUBO { float u_test_value; };
struct MaterialUBO { float3 color; };
struct Push { float4x4 model_mat; };

ConstantBuffer<GlobalUBO>   g_ubo    : register(b0, space0);
ConstantBuffer<ObjectUBO>   o_ubo    : register(b0, space1);
ConstantBuffer<MaterialUBO> m_ubo    : register(b0, space2);
[[vk::push_constant]] ConstantBuffer<Push> push : register(b0, space3);

struct VSOutput {
  float4 position : SV_POSITION;
  float3 color : COLOR0;
};

[shader("vertex")]
VSOutput main(float3 position : POSITION0) {
  VSOutput o;
  o.position = mul(g_ubo.view_projection,
                   mul(push.model_mat, float4(position, 1.0)));
  o.color = m_ubo.color;
  return o;
}
