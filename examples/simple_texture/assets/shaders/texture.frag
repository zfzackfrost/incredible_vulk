#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec2 texCoords;
layout(binding = 1) uniform sampler2D tex;

void main() {
    fColor = texture(tex, texCoords);
}
