#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 vertexColor;

layout (binding = 1) uniform sampler2D tex;

void main() {
    vec3 base = texture(tex, texCoords).rgb;
    fColor = vec4(vertexColor * base, 1.0);
}
