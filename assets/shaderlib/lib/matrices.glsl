{% macro declareUBO(binding) -%}
layout(binding = {{ binding }}) uniform MatricesUBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} matrices;
{%- endmacro %}
