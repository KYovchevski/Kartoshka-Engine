#version 450
#pragma shader_stage(fragment)


layout(binding = 0, set = 0) uniform sampler smp;

layout(binding = 1, set = 0) uniform texture2D tex;

layout(binding = 2, set = 0) uniform Material
{
	vec4 m_Diffuse;
} u_Material;

layout(binding = 0, set = 1) uniform PointLight
{
	vec3 m_Position;
	float m_Padding;
	vec3 m_Color;
} u_PointLight;

layout(location = 0) in	vec2 i_TexCoords;
layout(location = 1) in vec3 i_ViewPosition;
layout(location = 2) in vec4 i_Color;
layout(location = 3) in vec3 i_Normal;

layout(location = 0) out vec4 o_Diffuse;

void main()
{
	vec3 directionToLight = -normalize(i_ViewPosition - u_PointLight.m_Position);
	float lambert = dot(i_Normal, directionToLight);
	vec3 lambertColor = lambert * u_PointLight.m_Color;

	o_Diffuse = texture(sampler2D(tex, smp), i_TexCoords);
	o_Diffuse *= u_Material.m_Diffuse * i_Color * vec4(lambertColor, 1.0);
}
