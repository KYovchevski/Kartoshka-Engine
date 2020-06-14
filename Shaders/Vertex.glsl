#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec2 i_Tex;
layout (location = 1) in vec3 i_Pos;

layout(push_constant) uniform PushConstants {
	mat4 m_MVP;
} u_Push;

layout (location = 0) out vec2 o_Tex;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	o_Tex = i_Tex;

	gl_Position = u_Push.m_MVP * vec4(i_Pos, 1.0f);
}