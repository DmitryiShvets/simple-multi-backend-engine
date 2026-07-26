// f_triangle.hlsl
struct PSInput {
    float4 position  : SV_POSITION;
    float3 fragColor : ATTRIB1;
};
float4 PSMain(PSInput input) : SV_TARGET {
    return float4(input.fragColor, 1.);
}
