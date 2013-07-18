{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block extra_definitons %}

{% for method in private_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}


}

{% endwith %}
{% endfor %}

{% endblock extra_definitons %}

{% block function_definitions %}

{% for method in public_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for property in members %}

{{ property.type }} {{ name }}::{{ property.name }}() const
{
    return m_{{ property.name }};
}


void {{ name }}::set{{ property.name|upper_first }}({{ property.type|arg_type }} {{ property.name }})
{
    m_{{ property.name }} = {{ property.name }};
}

{% endfor %}

{% endblock function_definitions %}

{% block bottom %}
#include "{{ output_file_header|cut:".h"|cut:".hpp" }}.moc"
{% endblock bottom %}
