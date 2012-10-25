{% extends "cpp_header.h" %}
{% block forward_declarations %}
namespace Ui
{
class {{ name }};
}
{% endblock forward_declarations %}

{% block class_body %}
    Q_OBJECT
{{ block.super }}


private:
    Ui::{{ name }}* ui;
{% endblock class_body %}
