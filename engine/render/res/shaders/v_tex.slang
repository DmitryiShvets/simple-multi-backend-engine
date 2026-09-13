struct GlobalUBO {
  float4x4 view_projection;
  float3 light_position;
  float3 Kd;
  float3 Ld;
  float3 camera_position;
};
struct ObjectUBO { float u_test_value; };
struct MaterialUBO { float3 color; };

[[vk::binding(0, 0)]] ConstantBuffer<GlobalUBO> g_ubo;
[[vk::binding(0, 1)]] ConstantBuffer<ObjectUBO> o_ubo;
[[vk::binding(0, 2)]] ConstantBuffer<MaterialUBO> m_ubo;

struct Push { float4x4 model_mat; };
[[vk::push_constant]] ConstantBuffer<Push> model_mat;

struct VSOutput {
  float4 position : SV_POSITION;
  float2 texCoord : TEXCOORD0;
  float3 color : COLOR1;
};

[shader("vertex")]
VSOutput main(float3 position : POSITION0,
              float3 normal : NORMAL0,
              float2 texCoord : TEXCOORD0) {
  VSOutput o;
  o.position = mul(g_ubo.view_projection,
                   mul(model_mat.model_mat, float4(position, 1.0)));
  o.texCoord = texCoord;
  o.color = m_ubo.color;
  return o;
}
