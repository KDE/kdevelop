{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block extra_declarations %}

class {{ name }}Private
{
public:
    {% for property in members %}
    {{ property.type }} {{ property.name }};
    {% endfor %}

    {% for method in private_functions %}
    {% include "method_declaration.txt" %}
    {% endfor %}
};

{% endblock extra_declarations %}

{% block extra_definitons %}

{% with name|add:"Private" as name %}
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
{% endwith %}

{% endblock extra_definitons %}

{% block function_definitions %}

{% for method in public_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.is_constructor %}: d_ptr(new {{ name }}Private){% endif %}
{
    {% if method.isDestructor %}
    delete d_ptr;{% endif %}{% if method.type %}return {{ method.default_return_value }};
    {% endif %}


}

{% endwith %}
{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.is_constructor %}: d_ptr(new {{ name }}Private){% endif %}
{
    {% if method.isDestructor %}
    delete d_ptr;{% endif %}{% if method.type %}return {{ method.default_return_value }};
    {% endif %}


}

{% endwith %}
{% endfor %}

{% for property in members %}

{{ property.type }} {{ property.name }}() const
{
    Q_D(const {{ name }});
    return d->{{ property.name }};
}


void set{{ property.name|upper_first }}({{ property.type|arg_type }} {{ property.name }})
{
    Q_D({{ name }});
    d->{{ property.name }} = {{ property.name }};
}

{% endfor %}

{% endblock function_definitions %}

{% block bottom %}
#include "{{ output_file_header|cut:".h"|cut:".hpp" }}.moc"
{% endblock bottom %}