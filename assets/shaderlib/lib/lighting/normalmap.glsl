{% import "lib/library.glsl" as lib %}
{% call lib.new('lib_lighting_normalmap_common') %}

const vec3 FLAT_NORMAL = vec3(0.5, 0.5, 1.0);

// Flatten (amount is positive) or intensify (amount is negative) a tangent space
// normal vector
vec3 flattenNormal(vec3 normalSample, float amount)
{
    return mix(normalSample, FLAT_NORMAL, amount);
}

// World-space normals in the range [-1, 1] from a tangent-space normal vector in the range [0, 1]
vec3 normalsFromTangent01(vec3 normalTangentSpace, mat3 TBN)
{
    return normalize(TBN * (normalTangentSpace * 2.0 - 1.0));
}

{% endcall %}
