struct CameraData {
    float4x4 view_proj;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> camera;
[[vk::binding(1, 0)]] Sampler2D tex;

struct VSInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct VSOutput {
    float4 pos : SV_Position;
    float2 uv;
};

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput o;
    o.pos = mul(camera.view_proj, float4(input.pos, 0.0f, 1.0f));
    o.uv = input.uv;
    return o;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    return tex.Sample(input.uv);
}