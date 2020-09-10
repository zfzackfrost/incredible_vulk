{% extends "lib/base.glsl" %}

{% import "lib/lighting/common.glsl" as lighting %}

{% block preamble -%}

{{ lighting.common(1, pointLightsPerPass|default(4), dirLightsPerPass|default(4)) }}

layout(location = 0) in VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 texCoords; 
} fsIn;

layout (location = 0) out vec4 fColor;

float getAlpha()
{
    {% block alpha -%}
    return 1;
    {%- endblock %}
}
float getShininess()
{
    {% block shininess -%}
    return 8;
    {%- endblock %}
}

vec3 getDiffuseColor()
{
    {% block matDiffuse -%}
    return vec3(0.5);
    {%- endblock %}
}

vec3 getSpecularColor()
{
    {% block matSpecular -%}
    return vec3(1.0);
    {%- endblock %}
}
vec3 getAmbientAmount()
{
    {% block ambientAmount -%}
    return vec3(0.05);
    {%- endblock %}
}

{%- endblock %}



{% block main -%}
    
    vec3 N = normalize(fsIn.normal);
    vec3 V = normalize(lighting.viewPos - fsIn.position);
    float shininess = getShininess();

    vec3 specular = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 ambient = getAmbientAmount();
    for (int i = 0; i < lighting.pointLightCount; ++i)
    {
        PointLight light = lighting.pointLights[i];
        vec3 L = normalize(light.position - fsIn.position);
        vec3 H = normalize(V + L);

        float diff = max(dot(N, L), 0.0);
        diffuse += diff * light.color;

        float spec = pow(max(dot(N, H), 0.0), shininess);
        specular += spec * light.color;
    }
    for (int i = 0; i < lighting.dirLightCount; ++i)
    {
        DirectionLight light = lighting.dirLights[i];
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(V + L);

        float diff = max(dot(N, L), 0.0);
        diffuse += diff * light.color;

        float spec = pow(max(dot(N, H), 0.0), shininess);
        specular += spec * light.color;
    }
    
    fColor.rgb = ((ambient + diffuse) * getDiffuseColor()) + (specular * getSpecularColor());
    fColor.a = getAlpha();
{%- endblock %}
