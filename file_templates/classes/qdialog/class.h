{% extends "cpp_qobject_header.h" %}


{% block qobject_default_include %}
#include <QDialog>
{% endblock qobject_default_include %}


{% block includes %}
{{ block.super }}
#include <QScopedPointer>
{% endblock includes %}


{% block qobject_baseclass %}QDialog{% endblock qobject_baseclass %}


{% block forward_declarations %}
namespace Ui
{
class {{ name }};
}
{% endblock forward_declarations %}


{% block class_body %}
{{ block.super }}

{% if not private_members and not private_functions %}

private:
{% endif %}
    QScopedPointer<Ui::{{ name }}> m_ui;
{% endblock class_body %}
