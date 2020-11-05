#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 i_Pos;

layout(push_constant) uniform PushConstants 
{
	mat4 m_MVP; // Local to Clip
} u_Push;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	gl_Position = u_Push.m_MVP * vec4(i_Pos, 1.0f);
}