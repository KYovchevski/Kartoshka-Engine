#version 450
#pragma shader_stage(fragment)

layout(binding = 0, set = 0) uniform sampler smp;

layout(binding = 1, set = 0) uniform texture2D tex;

layout(binding = 2, set = 0) uniform Material
{
	vec4 m_Diffuse;
} u_Material;

layout(binding = 3, set = 0) uniform texture2D u_NormalMap;

struct PointLight
{
	vec3 m_Position;
	float m_Padding;
	vec3 m_Color;
	float m_Padding1;
};

layout(binding = 0, set = 1) buffer PointLights
{
	short int numLights;
	ivec3 _padding; // not required, as the compiler appears to add the padding automatically, but for consistency
	PointLight m_Lights[];
} b_PointLights;

layout(location = 0) in	vec2 i_TexCoords;
layout(location = 1) in vec3 i_WorldPosition;
layout(location = 2) in vec4 i_Color;
layout(location = 3) in vec3 i_Normal;
layout(location = 4) in mat3x3 i_TBN;

layout(location = 0) out vec4 o_Diffuse;

void main()
{	
	vec3 sampledNormal = texture(sampler2D(u_NormalMap, smp), vec2(i_TexCoords.x, i_TexCoords.y)).xyz;
	sampledNormal = sampledNormal * 2.0f - 1.0f;
	
	vec3 normal;
	normal = i_TBN * sampledNormal;
	normal = sampledNormal * i_TBN;

	vec4 diffuse = vec4(0.0f);
	for	(int i = 0; i < b_PointLights.numLights; i += 1)
	{
		vec3 toLight = b_PointLights.m_Lights[i].m_Position - i_WorldPosition;
		vec3 lightDirection = normalize(toLight);

		diffuse += dot(normal, lightDirection) * vec4(b_PointLights.m_Lights[i].m_Color, 1.0f);
	}

	diffuse = max(diffuse, 0.05f);

	o_Diffuse = texture(sampler2D(tex, smp), i_TexCoords);
	o_Diffuse *= u_Material.m_Diffuse * i_Color * diffuse;
}
