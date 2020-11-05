#version 450
#pragma shader_stage(fragment)

//precision highp float;
//precision highp samplerCubeShadow;

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
	int numLights;
	float m_Far;
	float m_Near;
	int padding[1];
	PointLight m_Lights[];
} b_PointLights;

layout(binding = 1, set = 1) uniform textureCube u_ShadowMap;
layout(binding = 2, set = 1) uniform sampler cmpSmp;

layout(location = 0) in	vec2 i_TexCoords;
layout(location = 1) in vec3 i_WorldPosition;
layout(location = 2) in vec4 i_Color;
layout(location = 3) in vec3 i_Normal;
layout(location = 4) in mat3x3 i_TBN;

layout(location = 0) out vec4 o_Diffuse;

float ProjectDistance(float a_Distance, float a_NearClip, float a_FarClip)
{
	float m22, m23, m32, m33;
	m33 = 0.0f;
	m23 = 1;

	m22 = (a_FarClip + a_NearClip) / (a_FarClip - a_NearClip);
	m32 = -(2 * a_FarClip * a_NearClip) / (a_FarClip - a_NearClip);

	float z, w;
	z = a_Distance * m22 + 1 * m32;
	w = a_Distance * m23 + 1 * m33;

	return z / w;
}

void main()
{	
	vec3 sampledNormal = texture(sampler2D(u_NormalMap, smp), vec2(i_TexCoords.x, i_TexCoords.y)).xyz;
	sampledNormal.y = 1.0f - sampledNormal.y;
	sampledNormal = sampledNormal * 2.0f - 1.0f;
	
	vec3 normal;
	//normal = i_TBN * sampledNormal;
	normal = sampledNormal * i_TBN;

	//normal = i_Normal;

	vec3 diffuse = vec3(0.0f);
	for	(int i = 0; i < b_PointLights.numLights; i += 1)
	{
		vec3 toLight = b_PointLights.m_Lights[i].m_Position - i_WorldPosition;

		vec3 sampleDir = normalize(toLight);
		sampleDir *= vec3(-1.0f, 1.0f, -1.0f);


		float distToLight;
		distToLight = length(toLight);
		distToLight = max(max(abs(toLight.x), abs(toLight.y)), abs(toLight.z));

		float shadowBias;


		float projectedDistance = ProjectDistance(distToLight - 0.05f, b_PointLights.m_Near, b_PointLights.m_Far);
		float shadowDepth = texture(samplerCube(u_ShadowMap, smp), sampleDir).r;
		//shadowBias = (1.0f - projectedDistance * 1.0001f);
		//shadowBias = 0.005f;
		//shadowBias = ProjectDistance(shadowBias, b_PointLights.m_Near, b_PointLights.m_Far);
		float depthTest = texture(samplerCubeShadow(u_ShadowMap, cmpSmp), vec4(sampleDir, projectedDistance)).r;

		float r;

		//r = 1.0f - step(shadowDepth + 0.005f, projectedDistance);

		r = pow(shadowDepth, 250.0f);

		//r = pow(shadowDepth, 30);
		//r = pow(projectedDistance, 30);

		r = depthTest;

		vec3 shadow = vec3(r, r, r);


		vec3 lightDirection = normalize(toLight);

		diffuse += clamp(dot(normal, lightDirection), 0.0f, 1.0f) * b_PointLights.m_Lights[i].m_Color * shadow;
		diffuse = shadow;
	}


	diffuse = clamp(diffuse, 0.3f, 1.0f);
	o_Diffuse = texture(sampler2D(tex, smp), i_TexCoords);
	o_Diffuse *= u_Material.m_Diffuse * i_Color * vec4(diffuse, 1.0f);
}
