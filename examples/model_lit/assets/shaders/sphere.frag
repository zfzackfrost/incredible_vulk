{% extends "lib/lighting/simple.frag" %}

{% block libs %}
    {{ super() }}
    {% include "lib/math.glsl" %}
{% endblock %}

{% block uniforms %}
    {{ super() }}
    {% call uniform.user(0) -%}sampler2D diffuseTex{%- endcall %}
    {% call uniform.user(1) -%}sampler2D specularTex{%- endcall %}
    {% call uniform.user(2) -%}sampler2D normalTex{%- endcall %}
{% endblock %}

{% block io %}
    {{ super() }}

    vec2 getTexCoords()
    {
        return fsIn.texCoords * 2.0;
    }
{% endblock %}

{% block post %}

{% include "lib/lighting/normalmap.glsl" %}

{{ super() }}
{% endblock %}

{% block shininess %}
    return 8.0;
{% endblock %}
{% block matNormal -%}
    vec3 norm = texture(normalTex, getTexCoords()).rgb;
    return flattenNormal(norm, -2.5);
{%- endblock %}
{% block matDiffuse -%}
    vec3 diff = texture(diffuseTex, getTexCoords()).rgb;
    return diff;
{%- endblock %}
{% block matSpecular -%}
    vec3 spec = texture(specularTex, getTexCoords()).rgb;
    return spec;
{%- endblock %}
