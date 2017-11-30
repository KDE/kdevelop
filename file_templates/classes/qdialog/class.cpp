{% extends "cpp_qobject_implementation.cpp" %}


{% block includes %}
{{ block.super }}
#include "ui_{{ output_file_ui|cut:".ui" }}.h"
{% endblock includes %}


{% block public_constructor_init_list %}
    : m_ui(new Ui::{{ name }})
{% endblock public_constructor_init_list %}


{% block public_constructor_body %}
{{ block.super }}
    m_ui->setupUi(this);
{% endblock public_constructor_body %}


{% block protected_constructor_init_list %}
    : m_ui(new Ui::{{ name }})
{% endblock protected_constructor_init_list %}


{% block protected_constructor_body %}
{{ block.super }}
    m_ui->setupUi(this);
{% endblock protected_constructor_body %}
