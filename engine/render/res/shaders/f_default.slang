[shader("fragment")]
float4 main(float4 position : SV_POSITION, float3 fragColor : COLOR0) : SV_TARGET0 {
  return float4(fragColor, 1.0);
}
