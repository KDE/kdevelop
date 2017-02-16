{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block extra_definitions %}

{% for method in private_functions %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endfor %}

{% endblock extra_definitions %}

{% block function_definitions %}

{# doing our own destructor #}
{{ name }}::~{{ name }}() = default;

{% for method in public_functions %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endfor %}

{% for method in protected_functions %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endfor %}

{% endblock function_definitions %}
