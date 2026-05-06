struct VSOutput {
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOutput main(uint vid : SV_VertexID) {
    static const float2 positions[3] = {
        float2( 0.0f, -0.5f),
        float2( 0.5f,  0.5f),
        float2(-0.5f,  0.5f),
    };
    static const float3 colors[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1),
    };
    VSOutput o;
    o.pos   = float4(positions[vid], 0.0f, 1.0f);
    o.color = colors[vid];
    return o;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    return float4(input.color, 1.0f);
}