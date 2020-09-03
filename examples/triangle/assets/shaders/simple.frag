#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexColor;
layout(location = 0) out vec4 fColor;

void main() {
    fColor = vec4(vertexColor, 1.0);
}
