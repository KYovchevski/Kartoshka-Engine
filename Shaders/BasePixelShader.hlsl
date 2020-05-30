
#pragma shader_stage(fragment)

struct VS_Output
{
    [[vk::location(0)]] float3 m_Color : COLOR;
};


float4 main(VS_Output a_Input) : SV_TARGET0
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}