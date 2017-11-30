{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}


{% block includes %}
{{ block.super }}
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
    {% with arguments|first as argFirst %}
    {# copy constructor? #}
    {% if arguments|length == 1 and argFirst.type == method.name|arg_type %}
    : d_ptr(new {{ name }}Private(*{{ argFirst.name }}.d_ptr))
    {% else %}
    : d_ptr(new {{ name }}Private())
    {% endif %}
    {% endwith %}
{% endif %}
{
{% block public_method_body %}
    {% if method.isConstructor %}
{% block public_constructor_body %}
{% endblock public_constructor_body %}
    {% elif method.isDestructor %}
{% block public_destructor_body %}
    delete d_ptr;
{% endblock public_destructor_body %}
    {% elif method.type %}
    return {{ method.default_return_value }};
    {% endif %}
{% endblock public_method_body %}
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

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
    {% with arguments|first as argFirst %}
    {# copy constructor? #}
    {% if arguments|length == 1 and argFirst.type == method.name|arg_type %}
    : d_ptr(new {{ name }}Private(*{{ argFirst.name }}.d_ptr))
    {% else %}
    : d_ptr(new {{ name }}Private())
    {% endif %}
    {% endwith %}
{% endif %}
{
{% block protected_method_body %}
   {% if method.isConstructor %}
{% block protected_constructor_body %}
{% endblock protected_constructor_body %}
    {% elif method.isDestructor %}
{% block protected_destructor_body %}
    delete d_ptr;
{% endblock protected_destructor_body %}
    {% elif method.type %}
    return {{ method.default_return_value }};
    {% endif %}
{% endblock protected_method_body %}
}

{% endwith %}
{% endfor %}

{% endblock function_definitions %}
