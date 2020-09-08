float saturate(float x) {
    return clamp(x, 0, 1);
}
vec2 saturate(vec2 x) {
    return clamp(x, vec2(0), vec2(1));
}
vec3 saturate(vec3 x) {
    return clamp(x, vec3(0), vec3(1));
}
vec4 saturate(vec4 x) {
    return clamp(x, vec4(0), vec4(1));
}
