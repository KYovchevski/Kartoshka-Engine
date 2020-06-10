#pragma shader_stage(vertex)

struct VS_Input
{
	[[vk::location(0)]]	float2 m_TexCoords;
    [[vk::location(1)]] float3 m_Position;
};

struct Offset
{
    float4 vec1;
};

[[vk::binding(0)]] Offset offset;

struct PushConstants
{
    float3 vec;
};

[[vk::push_constant]] PushConstants pushC;

struct VS_Output
{
    [[vk::location(0)]] float2 m_TexCoords : TEX;
    float4 m_Position : SV_Position;
};

VS_Output main(VS_Input a_Input)
{
    VS_Output output;

    output.m_Position = float4(a_Input.m_Position + offset.vec1 + pushC.vec, 1.0f);
    output.m_TexCoords = a_Input.m_TexCoords;
	
    return output;
}