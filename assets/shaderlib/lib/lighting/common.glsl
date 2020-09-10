{% macro common(uboIndex, pointLightsPerPass=4, dirLightsPerPass=4) -%}
#define POINT_LIGHTS_PER_PASS {{ pointLightsPerPass }}
#define DIR_LIGHTS_PER_PASS   {{ dirLightsPerPass }}

struct PointLight
{
    vec3 position;
    vec3 color;
};

struct DirectionLight
{
    vec3 direction;
    vec3 color;
};

layout (binding = {{ uboIndex }}) uniform LightingUBO {
    PointLight     pointLights[{{pointLightsPerPass}}];
    int pointLightCount;
    DirectionLight dirLights[{{dirLightsPerPass}}];
    int dirLightCount;
    vec3 viewPos;
} lighting;
{%- endmacro %}
