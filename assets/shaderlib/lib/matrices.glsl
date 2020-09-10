{% macro declareUBO(binding, enableNormal=false) -%}
#ifndef IVULK_LIB_MATRICESUBO
#define IVULK_LIB_MATRICESUBO
layout(binding = {{ binding }}) uniform MatricesUBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} matrices;
#endif
{%- endmacro %}
