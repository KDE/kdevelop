{% extends "cpp_header_onlyfunctions.h" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
#include <QSharedDataPointer>
{% endblock %}

{% block forward_declarations %}
class {{ name }}Data;
{% endblock forward_declarations %}

{% block class_body %}
{{ block.super }}
{% if members %}

public:
{% for property in members %}

    {% include "class_property_getter_declaration_apidox_cpp.txt" %}
    {% include "class_property_getter_declaration_cpp.txt" %}


    {% include "class_property_setter_declaration_apidox_cpp.txt" %}
    {% include "class_property_setter_declaration_cpp.txt" %}

{% endfor %}
{% endif %}

private:
    QSharedDataPointer<{{ name }}Data> d;
{% endblock class_body %}
