struct UniformBufferObject {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBufferObject> ubo;

struct VertexInput {
    [[vk::location(0)]] float2 pos;
    [[vk::location(1)]] float3 color;
};

struct VertexOutput {
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 0.0, 1.0))));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.color, 1.0);
}