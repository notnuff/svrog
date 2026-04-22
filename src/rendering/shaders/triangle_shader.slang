struct PushConstants {
    float time;
};

[[vk::push_constant]]
PushConstants pushConstants;

struct VertexInput {
    float2 pos;
    float3 color;
};

struct VertexOutput {
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    float cosA = cos(pushConstants.time);
    float sinA = sin(pushConstants.time);

    float2 rotatedPos;
    rotatedPos.x = input.pos.x * cosA - input.pos.y * sinA;
    rotatedPos.y = input.pos.x * sinA + input.pos.y * cosA;

    VertexOutput output;
    output.pos = float4(rotatedPos, 0.0, 1.0);
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.color, 1.0);
}