[[vk::binding(0, 3)]] Sampler2D g_shadow : register(t0, space3) : register(s0, space3);

[shader("fragment")]
float4 main(float4 position : SV_POSITION,
            float3 fragColor : COLOR0,
            float4 shadow_pos : TEXCOORD0,
            float shadow_depth_scale : TEXCOORD1,
            float shadow_uv_scale : TEXCOORD2
) : SV_TARGET0 {
  float3 ndc = shadow_pos.xyz / shadow_pos.w;
  float2 uv = float2(ndc.x * 0.5 + 0.5,  0.5 + (ndc.y * 0.5) * shadow_uv_scale);

  float current_depth =
      ndc.z * shadow_depth_scale + (1.0 - shadow_depth_scale);

  float shadow = 1.0;

  if (shadow_pos.w > 0.0 &&
      uv.x >= 0.0 && uv.x <= 1.0 &&
      uv.y >= 0.0 && uv.y <= 1.0) {
    float stored_depth = g_shadow.Sample(uv).r;
    if (current_depth - 0.002 > stored_depth)
      shadow = 0.35;
  }

  return float4(fragColor * shadow, 1.0);
}
