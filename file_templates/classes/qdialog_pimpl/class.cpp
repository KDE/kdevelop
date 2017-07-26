{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block includes %}
#include "{{ output_file_header }}"
#include "{{ output_file_privateheader }}"
{% endblock includes %}


{% block extra_definitions %}

{% for method in private_functions %}
{% with name|add:"Private" as name %}

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
{% if method.isConstructor %}
    : d_ptr(new {{ name }}Private())
{% endif %}
{
    {% if method.isConstructor %}
    d_ptr->ui.setupUi(this);
    {% endif %}
    {% if method.isDestructor %}
    delete d_ptr;
    {% endif %}
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
    : d_ptr(new {{ name }}Private())
{% endif %}
{
    {% if method.isConstructor %}
    d_ptr->ui.setupUi(this);
    {% endif %}
    {% if method.isDestructor %}
    delete d_ptr;
    {% endif %}
    {% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}

{% endwith %}
{% endfor %}

{% for property in members %}


{% include "class_property_getter_definition_cpp.txt" %}
{
    Q_D(const {{ name }});
    return d->{{ property.name }};
}


{% include "class_property_setter_definition_cpp.txt" %}
{
    Q_D({{ name }});
    if (d->{{ property.name }} == {{ property.name }}) {
        return;
    }


    d->{{ property.name }} = {{ property.name }};
    emit {{ property.name }}Changed(d->{{ property.name }});
}

{% endfor %}

{% endblock function_definitions %}
