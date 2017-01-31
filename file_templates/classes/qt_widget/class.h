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


{% if not private_members and not private_functions %}
private:
{% endif %}
    Ui::{{ name }}* ui;
{% endblock class_body %}
