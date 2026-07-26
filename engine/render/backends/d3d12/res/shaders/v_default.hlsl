// v_triangle.hlsl
cbuffer GlobalUBO  : register(b0, space0) { float4x4 projectionViewMatrix; };
cbuffer ObjectUBO  : register(b0, space1)  { float u_test_value; };
cbuffer MaterialUBO : register(b0, space2) { float3 color; };

cbuffer Push : register(b0, space3) { float4x4 model_mat; };

struct VSOutput {
    float4 position : SV_POSITION;
    float3 fragColor : ATTRIB1;
};

VSOutput VSMain(float3 position : ATTRIB0) {
    VSOutput o;
    o.position = mul(projectionViewMatrix, mul(model_mat, float4(position, 1.0)));
    o.fragColor = color;
    return o;
}
