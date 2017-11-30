{% extends "cpp_qobject_pimpl_p_header.h" %}


{% block includes %}
#include "ui_{{ output_file_ui|cut:".ui" }}.h"
{% endblock includes %}


{% block class_body %}
{{ block.super }}

    Ui::{{ name }} ui;
{% endblock class_body %}
