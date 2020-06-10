#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec2 inTex;
layout (location = 1) in vec3 inPos;

layout (binding = 0) uniform Offset
{
	vec3 vec1;
} offset;

layout(push_constant) uniform PushConstants {
	vec4 vec;
} pushConsts;

layout (location = 0) out vec2 outTex;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outTex = inTex;
	
	gl_Position = vec4(inPos.xyz + offset.vec1 + pushConsts.vec.xyz, 1.0);
	
}