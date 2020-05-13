#pragma shader_stage(vertex)


float4 positions[] =
{
    float4(0.0f, 0.5f, 0.0f, 1.0f),
    float4(0.5f, -0.5f, 0.0f, 1.0f),
    float4(-0.5f, -0.5f, 0.0f, 1.0f)
};

float4 main(uint a_VertexID : SV_VertexID) : SV_POSITION
{
	return positions[a_VertexID];
}