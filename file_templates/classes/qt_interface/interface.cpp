{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block extra_definitons %}

{% for method in private_functions %}
{% with method.arguments as arguments %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endwith %}
{% endfor %}

{% endblock extra_definitons %}

{% block function_definitions %}

{# doing our own destructor #}
{{ name }}::~{{ name }}() = default;

{% for method in public_functions %}
{% with method.arguments as arguments %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endwith %}
{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}
{# skipping any defined destructor #}
{% if not method.isDestructor %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endif %}
{% endwith %}
{% endfor %}

{% endblock function_definitions %}
