struct MaterialUBO { float3 color; };
[[vk::binding(0, 2)]]
ConstantBuffer<MaterialUBO> m_ubo;

[shader("fragment")]
float4 main(float3 fragColor : COLOR0) : SV_TARGET0 {
  return float4(fragColor, 1.0);
}
