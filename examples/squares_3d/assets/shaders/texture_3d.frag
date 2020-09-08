#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 tint;

layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D tex;

void main() {
    vec4 s = texture(tex, texCoords);
    fColor = vec4(s.rgb * tint, s.a);
}
