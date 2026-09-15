[[vk::binding(0, 3)]] Sampler2D g_t : register(t0, space3) : register(s0, space3);

[shader("fragment")]
float4 main(float4 position : SV_POSITION, float2 fragTexCoord : TEXCOORD0, float3 fragColor : COLOR1) : SV_TARGET0 {
  return g_t.Sample(fragTexCoord) * float4(fragColor, 1.0);
}
