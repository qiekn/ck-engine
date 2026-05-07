struct CameraData {
    float4x4 view_proj;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> camera;
[[vk::binding(1, 0)]] Sampler2D textures[];

struct VSInput {
    float3 position : POSITION;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD0;
    uint   tex_id   : TEXCOORD1;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD0;
    nointerpolation uint tex_id : TEXCOORD1;
};

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput o;
    o.position = mul(camera.view_proj, float4(input.position, 1.0f));
    o.color    = input.color;
    o.uv       = input.uv;
    o.tex_id   = input.tex_id;
    return o;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    return input.color * textures[NonUniformResourceIndex(input.tex_id)].Sample(input.uv);
}
