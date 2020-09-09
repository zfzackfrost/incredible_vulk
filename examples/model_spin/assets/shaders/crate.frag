#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D tex;

layout(location = 0) out vec4 fColor;
layout(location = 0) in VS_OUT {
    vec2 texCoords;
    vec3 normal;
    mat3 TBN;
} fsIn;

void main() {
    vec3 baseColor = texture(tex, fsIn.texCoords).rgb;
    fColor = vec4(baseColor, 1.0);
}
