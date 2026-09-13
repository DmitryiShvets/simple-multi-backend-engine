struct MaterialUBO { float3 color; };
[[vk::binding(0, 2)]]
ConstantBuffer<MaterialUBO> m_ubo;

[[vk::binding(0, 3)]]
Texture2D<float4> g_t;
[[vk::binding(0, 3)]]
SamplerState g_s;

[shader("fragment")]
float4 main(float2 fragTexCoord : TEXCOORD0, float3 fragColor : COLOR1) : SV_TARGET0 {
  return g_t.Sample(g_s, fragTexCoord) * float4(fragColor, 1.0);
}
