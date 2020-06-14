#version 450
#pragma shader_stage(fragment)

layout(binding = 0) uniform sampler smp;

layout(binding = 1) uniform texture2D tex;

layout(location = 0) in	vec2 i_TexCoords;

layout(location = 0) out vec4 o_Diffuse;

void main()
{
	o_Diffuse = texture(sampler2D(tex, smp), i_TexCoords);
}
