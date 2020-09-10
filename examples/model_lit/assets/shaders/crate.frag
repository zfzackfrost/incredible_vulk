{% extends "lib/lighting/simple.frag" %}
{% block preamble%}

{{ super() }}

{% endblock %}
{% block shininess %}
    return 4.0;
{% endblock %}
{% block matDiffuse -%}
    return vec3(0.1, 0.3, 0.55);
{%- endblock %}
{% block matSpecular -%}
    return vec3(0.1, 0.3, 0.55);
{%- endblock %}
