{% macro new(libName) %}
{% set inc_guard = "IVULK_" ~ (libName | upper()) ~ "_" ~ (libName | genid(16)) %}
#ifndef {{ inc_guard }}
#define {{ inc_guard }}
{{ caller() }}
#endif
{% endmacro %}
