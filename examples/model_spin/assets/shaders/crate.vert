#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec2 vTexCoords;

layout(location = 0) out VS_OUT
{
	vec2 texCoords;
	vec3 normal;
	mat3 TBN;
}
vsOut;

#include "crate.common.glsl"

void main()
{
	gl_Position = ubo.matrices.proj * ubo.matrices.view * ubo.matrices.model * vPosition;

	vsOut.texCoords = vTexCoords;

	vec3 T = normalize(vec3(ubo.matrices.model * vec4(vTangent, 0.0)));
	vec3 N = normalize(vec3(ubo.matrices.model * vec4(vNormal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	vsOut.TBN    = mat3(T, B, N);
	vsOut.normal = vNormal;
}
