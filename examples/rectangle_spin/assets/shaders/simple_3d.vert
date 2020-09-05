#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vColor;

layout(location = 0) out vec3 vertexColor;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vPosition;
    vertexColor = vColor;
}
