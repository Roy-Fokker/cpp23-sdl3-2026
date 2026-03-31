static const float2 positions[3] = {
	float2(0.0, -0.5),
	float2(0.5, 0.5),
	float2(-0.5, 0.5)
};

struct VSOutput
{
	float4 Position : SV_POSITION;
};

[shader("vertex")]
VSOutput vsMain(uint vertexId : SV_VertexID) {
	float2 pos = positions[vertexId % 3];
	VSOutput out;
	out.Position = float4(pos, 0.0, 1.0);
	return out;
}

[shader("fragment")]
float4 fsMain()
{
	return float4(1.0, 0.0, 0.0, 1.0);
}