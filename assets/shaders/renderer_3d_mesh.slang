struct CameraData {
    float4x4 view_proj;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> camera;

struct PushBlock {
    float4x4 model;
    float3   tint;
};
[[vk::push_constant]] ConstantBuffer<PushBlock> push;

struct VSInput {
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 normal_ws : NORMAL;
};

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput o;
    float4 world_pos = mul(push.model, float4(input.position, 1.0f));
    o.position = mul(camera.view_proj, world_pos);
    float3x3 model3 = float3x3(push.model[0].xyz,
                               push.model[1].xyz,
                               push.model[2].xyz);
    o.normal_ws = mul(model3, input.normal);
    return o;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    float3 N = normalize(input.normal_ws);
    // Hardcoded directional sun at "+up + +right + +front-toward-camera".
    float3 L = normalize(float3(0.4f, 0.8f, 0.5f));
    float NdotL = max(dot(N, L), 0.0f);
    float3 ambient = 0.2f * push.tint;
    float3 diffuse = NdotL * push.tint;
    return float4(ambient + diffuse, 1.0f);
}
