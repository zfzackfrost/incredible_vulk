#version 450 core
#extension GL_ARB_separate_shader_objects : enable

{% block preamble %}
{% endblock %}

void main()
{
    {% block main %}
    {% endblock %}
}
