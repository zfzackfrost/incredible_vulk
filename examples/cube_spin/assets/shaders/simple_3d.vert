#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vColor;

layout(location = 0) out vec3 vertexColor;

struct Matrices {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout (binding = 0) uniform UniformBufferObject {
    vec3 tint;
    Matrices matrices;
} ubo;

void main() {
    gl_Position = ubo.matrices.proj * ubo.matrices.view * ubo.matrices.model * vPosition;
    vertexColor = vColor * ubo.tint;
}
