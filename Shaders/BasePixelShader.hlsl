#pragma shader_stage(fragment)

[[vk::binding(1,0)]] sampler samp;

[[vk::binding(2,0)]] Texture2D tex;

struct VS_Output
{
	[[vk::location(0)]]	float2 m_TexCoords : TEX;
};


float4 main(VS_Output a_Input) : SV_TARGET0
{
	//return float4(1.0f, 0.0f, 1.0f, 1.0f);
	return float4(tex.Sample(samp, a_Input.m_TexCoords));
}