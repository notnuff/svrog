struct UniformBufferObject {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBufferObject> ubo;

[[vk::binding(0, 1)]]
Sampler2D texSampler;

struct PushConstants {
    float4x4 model;
    float time;
};

[[vk::push_constant]]
PushConstants pc;

struct VertexInput {
    [[vk::location(0)]] float3 pos;
    [[vk::location(1)]] float3 normal;
    [[vk::location(2)]] float2 texCoord;
};

struct VertexOutput {
    float4 pos : SV_Position;
    float3 normal;
    float2 texCoord;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(pc.model, float4(input.pos, 1.0))));
    output.normal = mul((float3x3)pc.model, input.normal);
    output.texCoord = input.texCoord;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return texSampler.Sample(inVert.texCoord);
}