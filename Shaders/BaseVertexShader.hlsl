#pragma shader_stage(vertex)

//static const float4 positions[] =
//{
//    float4(0.0f, 0.5f, 0.0f, 1.0f),
//    float4(-0.5f, -0.25f, 0.0f, 1.0f),
//    float4(0.5f, -0.25f, 0.0f, 1.0f)
//};


struct VS_Input
{
    [[vk::location(0)]] float3 m_Color;
    [[vk::location(1)]] float3 m_Position;
};

struct VS_Output
{
    [[vk::location(0)]] float3 m_Color;
    float4 m_Position : SV_Position;
};

VS_Output main(VS_Input a_Input, uint a_VertexIndex : SV_VertexID )
{
    VS_Output output;
    
    output.m_Color = a_Input.m_Color;
    output.m_Position = float4(a_Input.m_Position, 1.0f);
    
    return output;
}