struct Matrices {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout (binding = 0) uniform UniformBufferObject {
    vec3 tint;
    Matrices matrices;
} ubo;
