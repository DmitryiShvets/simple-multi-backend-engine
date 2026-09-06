// f_triangle.hlsl
struct PSInput {
    float4 position  : SV_POSITION;
    float2 fragTexCoord : ATTRIB1;
    float3 fragColor : ATTRIB2;
};

Texture2D    g_texture : register(t0, space3);
SamplerState g_sampler : register(s0, space3);

float4 PSMain(PSInput input) : SV_TARGET {
    return g_texture.Sample(g_sampler, input.fragTexCoord) * float4(input.fragColor, 1.);
}
