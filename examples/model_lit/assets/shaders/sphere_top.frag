{% extends "lib/lighting/simple.frag" %}
{% block uniforms %}
    {{ super() }}
{% endblock %}

{% block io %}
    {{ super() }}
{% endblock %}

{% block shininess %}
    return 8.0;
{% endblock %}
{% block matDiffuse -%}
    return vec3(1.0, 0.05, 0.05);
{%- endblock %}
{% block matSpecular -%}
    return vec3(0.55);
{%- endblock %}
