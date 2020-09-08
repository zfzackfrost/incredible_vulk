#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec2 vTexCoords;
layout (location = 2) in vec3 vColor;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec3 vertexColor;

#include "cube.common.glsl"
#include "lib/math.glsl"

void main() {
    gl_Position = ubo.matrices.proj * ubo.matrices.view * ubo.matrices.model * vPosition;
    vertexColor = saturate(vColor * ubo.tint);
    texCoords = vTexCoords;
}