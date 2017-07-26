{% extends "cpp_header.h" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
{% if not base_classes %}
#include <QObject>
{% endif %}
{% endblock includes %}


{% block forward_declarations %}
class {{ name }}Private;
{% endblock forward_declarations %}


{% block class_declaration_open %}
{% if base_classes %}
{{ block.super }}
{% else %}
{% include "class_declaration_apidox_cpp.txt" %}
class {{ name }} : public QObject
{
{% endif %}
{% endblock class_declaration_open %}

{% block class_body %}
    Q_OBJECT
    {% for property in members %}
    {% include "class_qproperty_declaration_cpp.txt" %}
    {% endfor %}


{% if public_functions or members %}
public:
{% endif %}
    {% for method in public_functions %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

    {% endfor %}

    {% for property in members %}

    {% include "class_property_getter_declaration_apidox_cpp.txt" %}
    {% include "class_property_getter_declaration_cpp.txt" %}

    {% endfor %}


{% if members %}
public Q_SLOTS:
    {% for property in members %}

    {% include "class_property_setter_declaration_apidox_cpp.txt" %}
    {% include "class_property_setter_declaration_cpp.txt" %}

    {% endfor %}

Q_SIGNALS:
    {% for property in members %}

    {% include "class_property_signal_declaration_cpp.txt" %}

    {% endfor %}
{% endif %}


{% if protected_functions %}
protected:
    {% for method in protected_functions %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

    {% endfor %}
{% endif %}


private:
    {{ name }}Private* const d_ptr;
    Q_DECLARE_PRIVATE({{ name }})
{% endblock class_body %}
