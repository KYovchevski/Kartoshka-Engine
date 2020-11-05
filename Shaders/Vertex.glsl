#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 i_Pos;
layout (location = 1) in vec2 i_Tex;
layout (location = 2) in vec4 i_Color;
layout (location = 3) in vec3 i_Normal;
layout (location = 4) in vec4 i_Tangent;

layout(push_constant) uniform PushConstants 
{
	mat4 m_WorldMatrix; // Local to View 

	mat4 m_MVP; // Local to Clip
} u_Push;

layout (location = 1) out vec3 o_WorldPosition;
layout (location = 0) out vec2 o_Tex;
layout (location = 2) out vec4 o_Color;
layout (location = 3) out vec3 o_Normal;
layout (location = 4) out mat3x3 o_TBN;

out gl_PerVertex
{
	vec4 gl_Position;
};

mat3x3 CalculateTBN(vec4 a_Tangent, vec3 a_Normal)
{

	mat4 mat = inverse(transpose(u_Push.m_WorldMatrix));

	// Transform the normal into world space
	vec3 N = normalize(vec4(a_Normal, 0.0f) * mat).xyz;
	
	// homogenize the tangent 
	vec3 T = normalize(a_Tangent.xyz);
	// Transform the tangent into world space
	T = normalize(vec4(T, 0.0f) * mat).xyz;

	vec3 B = cross(T, N) * a_Tangent.w;

	// N - world space normal
	// T - world space tangent

	// tangent space to world space?
//	return mat3x3(mat4(vec4(T, 0.0f), vec4(B, 0.0f), vec4(N, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	return inverse(mat3x3(T, B, N));
}

void main() 
{
	o_Tex = i_Tex;
	o_Color = i_Color;
	o_WorldPosition = (u_Push.m_WorldMatrix * vec4(i_Pos, 1.0f)).xyz;
	o_Normal = normalize(vec4(i_Normal, 0.0f) * inverse((u_Push.m_WorldMatrix))).xyz;
	o_TBN = CalculateTBN(i_Tangent, i_Normal);

	gl_Position = u_Push.m_MVP * vec4(i_Pos, 1.0f);
}