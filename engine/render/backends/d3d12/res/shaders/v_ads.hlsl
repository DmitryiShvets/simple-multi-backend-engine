// v_triangle.hlsl
cbuffer GlobalUBO : register(b0, space0) {
    float4x4 view_proj_mat;
    float3 light_pos;
    float _pad0;
    float3 Kd;
    float _pad1;
    float3 Ld;
    float _pad2;
    float3 camera_pos;
    float _pad3;
};

cbuffer ObjectUBO : register(b0, space1) {
    float4x4 model_matrix;
    float3 normal_col0;
    float _pad_obj0;
    float3 normal_col1;
    float _pad_obj1;
    float3 normal_col2;
    float _pad_obj2;
};

cbuffer MaterialUBO : register(b0, space2) {
    float3 color;
    float _pad_mat0;
};

cbuffer Push : register(b0, space3) {
    float4x4 model_mat;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 fragColor : ATTRIB1;
};

VSOutput VSMain(float3 position : ATTRIB0, float3 normal : ATTRIB1) {
    VSOutput o;

    float4 world_pos = mul(model_mat, float4(position, 1.0));
    o.position = mul(view_proj_mat, world_pos);

    float3x3 normal_mat = float3x3(normal_col0, normal_col1, normal_col2);
    float3 t_norm = normalize(mul(normal_mat, normal));

    float3 light_dir = normalize(light_pos - world_pos.xyz);
    float3 lit = Ld * Kd * max(dot(light_dir, t_norm), 0.0);
    o.fragColor = color * lit;

    return o;
}
