#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec2 i_Tex;
layout (location = 1) in vec3 i_Pos;
layout (location = 2) in vec4 i_Color;
layout (location = 3) in vec3 i_Normal;

layout(push_constant) uniform PushConstants {
	mat4 m_WorldViewMatrix; // Local to View 
	mat4 m_MVP; // Local to Clip
} u_Push;

layout (location = 0) out vec2 o_Tex;
layout (location = 1) out vec3 o_ViewPosition;
layout (location = 2) out vec4 o_Color;
layout (location = 3) out vec3 o_Normal;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	o_Tex = i_Tex;
	o_Color = i_Color;
	o_ViewPosition = (vec4(i_Pos, 1.0f) * u_Push.m_WorldViewMatrix).xyz;
	o_Normal = normalize(vec4(i_Normal, 0.0f) * inverse(transpose(u_Push.m_WorldViewMatrix)) ).xyz;
	//o_Normal = normalize(i_Normal);

	gl_Position = u_Push.m_MVP * vec4(i_Pos, 1.0f);
}