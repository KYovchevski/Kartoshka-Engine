#version 450
#pragma shader_stage(fragment)


layout(binding = 0, set = 0) uniform sampler smp;

layout(binding = 1, set = 0) uniform texture2D tex;

layout(binding = 2, set = 0) uniform Material
{
	vec4 m_Diffuse;
} u_Material;

layout(binding = 3, set = 0) uniform texture2D u_NormalMap;

layout(binding = 0, set = 1) uniform PointLight
{
	vec3 m_Position;
	float m_Padding;
	vec3 m_Color;
} u_PointLight;

layout(location = 0) in	vec2 i_TexCoords;
layout(location = 1) in vec3 i_WorldPosition;
layout(location = 2) in vec4 i_Color;
layout(location = 3) in vec3 i_Normal;
layout(location = 4) in mat3x3 i_TBN;

layout(location = 0) out vec4 o_Diffuse;

void main()
{	
	vec3 sampledNormal = texture(sampler2D(u_NormalMap, smp), i_TexCoords).xyz;
	sampledNormal = sampledNormal * 2.0f - 1.0f;

	vec3 normal;
	normal = normalize(sampledNormal * i_TBN);

	vec3 toLight = u_PointLight.m_Position - i_WorldPosition;
	vec3 lightDirection = normalize(toLight);

	float diffuse = dot(normal, lightDirection);

	o_Diffuse = texture(sampler2D(tex, smp), i_TexCoords);
	o_Diffuse *= u_Material.m_Diffuse * i_Color * diffuse * vec4(u_PointLight.m_Color, 1.0f);
}
