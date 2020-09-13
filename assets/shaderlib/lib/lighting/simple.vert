{% extends "lib/base.vert" %}

{% import "lib/mesh.glsl" as mesh %}

{% block uniforms -%}
{%- endblock %}

{% block io -%}
{{ mesh.staticMeshAttribs() }}

layout(location = 0) out VS_OUT {
    vec3 position;
    vec2 texCoords;
    mat3 TBN;
} vsOut;
{%- endblock %}

{% block main -%}
    mat4 modelMat = getModelMat();
    mat4 viewMat = getViewMat();
    mat4 projMat = getProjMat();
    vec4 pos = modelMat * vPosition;
    gl_Position = projMat * viewMat * pos;
    vsOut.position = pos.xyz;
    vsOut.texCoords = vTexCoords;

    // ====== Tangent-Bitangent-Normal Matrix ====== //
    
    vec3 T = normalize(vec3(modelMat * vec4(vTangent, 0.0)));
    vec3 N = normalize(vec3(modelMat * vec4(vNormal, 0.0)));
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(N, T);
    vsOut.TBN = mat3(T, B, N);

{%- endblock %}
