{% extends "cpp_header.h" %}
{% load kdev_filters %}


{% block include_guard_open %}
{% with "_P_H" as include_guard_suffix %}
#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}
{% endwith %}
{% endblock include_guard_open %}


{% block includes %}
#include "ui_{{ output_file_ui|cut:".ui" }}.h"
{% endblock includes %}


{% block class_declaration_open %}
{% include "class_declaration_apidox_cpp.txt" %}
class {{ name }}Private
{
{% endblock class_declaration_open %}

{% block class_body %}
public:
    {% for method in private_functions %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

    {% endfor %}

    {% for property in members %}
    {{property.type}} {{property.name}};

    {% endfor %}

    Ui::{{ name }} ui;
{% endblock class_body %}

{% block include_guard_close %}
{% with "_P_H" as include_guard_suffix %}
#endif // {% include "include_guard_cpp.txt" %}
{% endwith %}
{% endblock include_guard_close %}
