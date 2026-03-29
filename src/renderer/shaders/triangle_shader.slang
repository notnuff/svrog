
static float2 triScreenPositions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

static float3 triColors[3] = {
    float3(0.0, 0.0, 1.0),
    float3(0.0, 1.0, 0.0),
    float3(1.0, 0.0, 0.0)
};

struct TriVertex {
    float4 pos;
    float4 color;
};

[shader("vertex")]
TriVertex vertMain(uint vertexIdx : SV_VertexID) {

    TriVertex output;
    output.pos = float4(triScreenPositions[vertexIdx], 0.0, 1.0);
    output.color = float4(triColors[vertexIdx], 1.0);

    return output;
}

[shader("fragment")]
float4 fragMain(TriVertex inVert) : SV_Target
{
    return inVert.color;
}