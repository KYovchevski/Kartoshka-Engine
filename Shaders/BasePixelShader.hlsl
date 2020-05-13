
#pragma shader_stage(fragment)

struct VS_Output
{
    [[vk::location(0)]] float3 m_Color : COLOR;
};


float4 main(VS_Output a_Input) : SV_TARGET
{
	return float4(a_Input.m_Color, 1.0f);
}