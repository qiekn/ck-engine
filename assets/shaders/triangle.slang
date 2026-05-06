struct VSInput {
    float2 pos   : POSITION;
    float3 color : COLOR;
};

struct VSOutput {
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput o;
    o.pos = float4(input.pos, 0.0f, 1.0f);
    o.color = input.color;
    return o;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    return float4(input.color, 1.0f);
}