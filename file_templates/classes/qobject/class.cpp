{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block extra_definitions %}

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

{% endblock extra_definitions %}

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


{% include "class_property_getter_definition_cpp.txt" %}
{
    return m_{{ property.name }};
}


{% include "class_property_setter_definition_cpp.txt" %}
{
    if (m_{{ property.name }} == {{ property.name }}) {
        return;
    }


    m_{{ property.name }} = {{ property.name }};
    emit {{ property.name }}Changed(m_{{ property.name }});
}

{% endfor %}

{% endblock function_definitions %}

{% block bottom %}
{% endblock bottom %}
