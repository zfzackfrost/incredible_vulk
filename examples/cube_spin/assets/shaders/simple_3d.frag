#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexColor;
layout(location = 0) out vec4 fColor;

layout (binding = 0) uniform UniformBufferObject {
    vec3 tint;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    fColor = vec4(ubo.tint, 1) * vec4(vertexColor, 1.0);
}
