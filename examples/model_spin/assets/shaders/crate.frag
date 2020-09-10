{% extends "lib/lighting/simple.frag" %}
{% block preamble%}

{{ super() }}

{% endblock %}
{% block matDiffuse -%}
    return vec3(0.2, 0.5, 1.0);
{%- endblock %}
