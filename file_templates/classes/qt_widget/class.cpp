{% extends "cpp_implementation.cpp" %}

{% block includes %}
#include "{{ output_file_header }}"
#include "ui_{{ output_file_ui|cut:".ui" }}.h"
{% endblock includes %}

{% block function_definitions %}
{% for method in functions %}

{% with method.internal_methods as arguments %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.isConstructor %}
    ui = new Ui::{{ name }};
    ui->setupUi(this);
    {% endif %}
    {% if method.isDestructor %}
    delete ui;
    {% endif %}
}

{% endwith %}
{% endfor %}
{% endblock function_definitions %}
