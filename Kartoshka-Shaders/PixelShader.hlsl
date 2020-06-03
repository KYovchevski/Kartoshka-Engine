#pragma shader_stage(fragment)

[[vk::binding(1)]] sampler samp;

[[vk::binding(2)]] Texture2D texture;

struct VS_Output
{
    [[vk::location(0)]] float3 m_Color : COLOR;
    [[vk::location(1)]]	float2 m_TexCoords : TEX;
};


float4 main(VS_Output a_Input) : SV_TARGET0
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return float4(texture.Sample(samp, a_Input.m_TexCoords));
}