#version 450 core
#extension GL_ARB_separate_shader_objects : enable

{% import "lib/uniform.glsl" as uniform %}

{% block libs %}
{% endblock %}

{% block uniforms %}
{% endblock %}

{% block pre %}
{% endblock %}

{% block io %}
{% endblock %}

{% block funcs %}
{% endblock %}

{% block post %}
{% endblock %}

void main()
{
    {% block main %}
    {% endblock %}
}
