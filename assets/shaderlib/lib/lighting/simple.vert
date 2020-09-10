{% extends "lib/base.glsl" %}

{% import "lib/mesh.glsl" as mesh %}
{% import "lib/matrices.glsl" as matrices %}

{% block preamble -%}
{{ matrices.declareUBO(0) }}

{{ mesh.staticMeshAttribs() }}

layout(location = 0) out VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 texCoords; 
} vsOut;
{%- endblock %}

{% block main -%}
    vec4 pos = matrices.model * vPosition;
    gl_Position = matrices.proj * matrices.view * pos;
    vsOut.position = pos.xyz;
    vsOut.texCoords = vTexCoords;
    vsOut.normal = mat3(transpose(inverse(matrices.model))) * vNormal;
{%- endblock %}
