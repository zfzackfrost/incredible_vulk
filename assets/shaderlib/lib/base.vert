#version 450 core
#extension GL_ARB_separate_shader_objects : enable

{% import "lib/uniform.glsl" as uniform %}

layout(push_constant) uniform MatricesPushConstants {
    mat4 model;
} matrices;

layout(binding = 0) uniform MatricesUbo {
    mat4 view;
    mat4 proj;
} matricesBuf;

mat4 getModelMat()
{
    return matrices.model;
}
mat4 getViewMat()
{
    return matricesBuf.view;
}
mat4 getProjMat()
{
    return matricesBuf.proj;
}

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
