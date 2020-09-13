{% extends "lib/base.frag" %}
{% set fn = "_" ~ ("simple_lighting_fn" | genid()) %}


{% import "lib/lighting/common.glsl" as lighting %}

{% block pre -%}

{{ lighting.common(1, pointLightsPerPass|default(4), dirLightsPerPass|default(4)) }}

{% endblock %}

{% block io -%}
layout (location = 0) out vec4 fColor;
layout(location = 0) in VS_OUT {
    vec3 position;
    vec2 texCoords; 
    mat3 TBN;
} fsIn;

{%- endblock %}

{% block post %}

float getAlpha{{fn}}()
{
    {% block alpha -%}
    return 1;
    {%- endblock %}
}
float getShininess{{fn}}()
{
    {% block shininess -%}
    return 8;
    {%- endblock %}
}

vec3 getNormal{{fn}}()
{
    {% block matNormal -%}
    return vec3(0.5, 0.5, 1);
    {%- endblock %}
}
vec3 getDiffuseColor{{fn}}()
{
    {% block matDiffuse -%}
    return vec3(0.5);
    {%- endblock %}
}

vec3 getSpecularColor{{fn}}()
{
    {% block matSpecular -%}
    return vec3(1.0);
    {%- endblock %}
}
vec3 getAmbientAmount{{fn}}()
{
    {% block ambientAmount -%}
    return vec3(0.05);
    {%- endblock %}
}

{%- endblock %}



{% block main -%}
    
    vec3 N = getNormal{{fn}}() * 2.0 - 1.0;
    N = normalize(fsIn.TBN * N);
    vec3 V = normalize(lighting.viewPos - fsIn.position);
    float shininess = getShininess{{fn}}();

    vec3 specular = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 ambient = getAmbientAmount{{fn}}();
    for (int i = 0; i < pointLightCount(); ++i)
    {
        PointLight light = lighting.pointLights[i];

        // Light direction
        vec3 L = normalize(light.position - fsIn.position);

        // Halfway vector
        vec3 H = normalize(V + L);

        // calculate attenuation
        float attenuation = pointLightAttenuation(light, fsIn.position);

        float diff = max(dot(N, L), 0.0) * attenuation;
        diffuse += diff * light.color;

        float spec = pow(max(dot(N, H), 0.0), shininess) * attenuation;
        specular += spec * light.color;
    }
    for (int i = 0; i < dirLightCount(); ++i)
    {
        DirectionLight light = lighting.dirLights[i];
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(V + L);

        float diff = max(dot(N, L), 0.0);
        diffuse += diff * light.color;

        float spec = pow(max(dot(N, H), 0.0), shininess);
        specular += spec * light.color;
    }
    
    fColor.rgb = ((ambient + diffuse) * getDiffuseColor{{fn}}()) + (specular * getSpecularColor{{fn}}());
    fColor.a = getAlpha{{fn}}();
{%- endblock %}
