#ifndef IVULK_LIB_MATH
#define IVULK_LIB_MATH

/////////////////////////////////////////////////////////////////////////////////
//                                  Saturate                                   //
/////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////
//                                   Fresnel                                   //
/////////////////////////////////////////////////////////////////////////////////

float fresnel(vec3 cameraToFrag, vec3 normalWorld, float bias, float scale, float power)
{
    return bias + scale * pow(1.0 + dot(cameraToFrag, normalWorld), power);
}

#endif
