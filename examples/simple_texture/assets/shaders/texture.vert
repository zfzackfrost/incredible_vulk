#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec2 vTexCoords;

layout(location = 0) out vec2 texCoords;

void main() {
    gl_Position = vPosition;
    texCoords = vTexCoords;
}
