#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec2 i_Tex;
layout (location = 1) in vec3 i_Pos;
layout (location = 2) in vec4 i_Color;
layout (location = 3) in vec3 i_Normal;
layout (location = 4) in vec4 i_Tangent;

layout(push_constant) uniform PushConstants 
{
	mat4 m_WorldMatrix; // Local to View 

	mat4 m_MVP; // Local to Clip
} u_Push;

layout (location = 0) out vec2 o_Tex;
layout (location = 1) out vec3 o_WorldPosition;
layout (location = 2) out vec4 o_Color;
layout (location = 3) out vec3 o_Normal;
layout (location = 4) out mat3x3 o_TBN;

out gl_PerVertex
{
	vec4 gl_Position;
};

mat3x3 CalculateTBN(vec4 a_Tangent, vec3 a_Normal)
{
	mat4 WorldToLocal = inverse(u_Push.m_WorldMatrix);

	vec3 T = normalize((vec4(a_Tangent.xyz, 0.0f) * WorldToLocal).xyz);
	vec3 N = normalize((vec4(a_Normal, 0.0f) *		WorldToLocal).xyz);

	vec3 B = -cross(T, N);

	return inverse(mat3x3(T, B, N));
}

void main() 
{

	o_Tex = i_Tex;
	o_Color = i_Color;
	o_WorldPosition = (vec4(i_Pos, 1.0f) * inverse(u_Push.m_WorldMatrix)).xyz;
	o_Normal = normalize(vec4(i_Normal, 0.0f) * inverse((u_Push.m_WorldMatrix))).xyz;
	//o_Normal = i_Normal;
	o_TBN = CalculateTBN(i_Tangent, i_Normal);


	gl_Position = u_Push.m_MVP * vec4(i_Pos, 1.0f);
}