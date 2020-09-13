{% import "lib/library.glsl" as lib %}

{% macro common(uboIndex, pointLightsPerPass=4, dirLightsPerPass=4) -%}
{% call lib.new('lib_lighting_common') %}
#define POINT_LIGHTS_PER_PASS {{ pointLightsPerPass }}
#define DIR_LIGHTS_PER_PASS   {{ dirLightsPerPass }}

struct PointLight
{
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionLight
{
    vec3 direction;
    vec3 color;
};

layout (binding = {{ uboIndex }}) uniform SceneUBO {
    PointLight     pointLights[{{pointLightsPerPass}}];
    DirectionLight dirLights[{{dirLightsPerPass}}];

    int pointLightCount;
    int dirLightCount;

    vec3 viewPos;
} scene;

int pointLightCount()
{
    return min(scene.pointLightCount, POINT_LIGHTS_PER_PASS);
}

int dirLightCount()
{
    return min(scene.dirLightCount, DIR_LIGHTS_PER_PASS);
}

float pointLightAttenuation(PointLight light, vec3 fragPos)
{
    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * d + 
            light.quadratic * (d * d));
    return attenuation;
}
{% endcall %}
{%- endmacro %}
