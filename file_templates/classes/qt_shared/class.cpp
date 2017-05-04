{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
#include <QSharedData>
{% endblock includes %}

{% block extra_declarations %}
class {% if namespaces %}{{ namespaces|join:"::" }}::{% endif %}{{ name }}Data : public QSharedData
{
public:
{% for member in members %}
    {{ member.type }} {{ member.name }};
{% endfor %}
};


{% for method in private_functions %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endfor %}

{% endblock extra_declarations %}

{% block function_definitions %}

{% for method in public_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
    {% with arguments|first as argFirst %}
    {# copy constructor? #}
    {% if arguments|length == 1 and argFirst.type == method.name|arg_type %}
    : d({{ argFirst.name }}.d)
    {% else %}
    : d(new {{ name }}Data())
    {% endif %}
    {% endwith %}
{% endif %}
{
    {% if "operator=" == method.name %}
    d = other.d;
    return *this;
    {% else %}
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
    {% with arguments|first as argFirst %}
    {# copy constructor? #}
    {% if arguments|length == 1 and argFirst.type == method.name|arg_type %}
    : d({{ argFirst.name }}.d)
    {% else %}
    : d(new {{ name }}Data())
    {% endif %}
    {% endwith %}
{% endif %}
{
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for property in members %}


{% include "class_property_getter_definition_cpp.txt" %}
{
    return d->{{ property.name }};
}


{% include "class_property_setter_definition_cpp.txt" %}
{
    d->{{ property.name }} = {{ property.name }};
}

{% endfor %}

{% endblock function_definitions %}
