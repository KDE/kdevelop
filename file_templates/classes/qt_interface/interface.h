{% extends "cpp_header.h" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
{% if not base_classes %}
#include <QObject>
{% endif %}
{% endblock %}

{% block class_body %}
public:
{# doing our own destructor, to ensure there is one #}
{% if not base_classes %}
    virtual ~{{ name }}();
{% else %}{# Assumption: subclassing only other interfaces #}
    ~{{ name }}() override;
{% endif %}

    {% for method in public_functions %}
        {# skipping any defined destructor #}
        {% if not method.isDestructor %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

        {% endif %}
    {% endfor %}

{% for property in members %}

    {% include "class_property_getter_declaration_apidox_cpp.txt" %}
    virtual {{ property.type }} {{ property.name }}() const = 0;


    {% include "class_property_setter_declaration_apidox_cpp.txt" %}
    virtual void set{{ property.name|upper_first }}({{ property.type|arg_type }} {{ property.name }}) = 0;

{% endfor %}
{% if protected_functions %}

protected:
    {% for method in protected_functions %}
        {# skipping any defined destructor #}
        {% if not method.isDestructor %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

        {% endif %}
    {% endfor %}
{% endif %}
{% if private_functions %}

private:
    {% for method in private_functions %}
    {# skipping any defined destructor #}
    {% if not method.isDestructor %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

    {% endif %}
    {% endfor %}
{% endif %}
{% endblock class_body %}

{% block outside_namespace %}
{{ block.super }}
Q_DECLARE_INTERFACE({% if namespaces %}{{ namespaces|join:"::" }}::{% endif %}{{ name }}, "{{ interfaceid }}")
{% endblock %}
