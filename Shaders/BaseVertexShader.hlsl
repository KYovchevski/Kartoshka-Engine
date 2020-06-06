#pragma shader_stage(vertex)

[[vk::binding(0)]] float3 Offset;

struct VS_Input
{
	[[vk::location(0)]]	float2 m_TexCoords;
    [[vk::location(1)]] float3 m_Position;
};

struct VS_Output
{
	[[vk::location(0)]] float2 m_TexCoords : TEX;
    float4 m_Position : SV_Position;
};

VS_Output main(VS_Input a_Input)
{
    VS_Output output;
    
    output.m_Position = float4(a_Input.m_Position + Offset, 1.0f);
    output.m_TexCoords = a_Input.m_TexCoords;

    output.m_Position *= float4(1.0f, -1.0f, 1.0f, 1.0f);
	
    return output;
}