{% extends "cpp_header.h" %}

{% block includes %}
{{ block.super }}
{% if not base_classes %}
{% block qobject_default_include %}
{% endblock qobject_default_include %}
{% endif %}
{% endblock includes %}


{% block class_declaration_open %}
{% if base_classes %}
{{ block.super }}
{% else %}
{% include "class_declaration_apidox_cpp.txt" %}
class {{ name }} : public {% block qobject_baseclass %}QObject{% endblock qobject_baseclass %}
{
{% endif %}
{% endblock class_declaration_open %}

{% block class_body %}
{% block qobject_body_header %}
    Q_OBJECT
    {% for property in members %}
    {% include "class_qproperty_declaration_cpp.txt" %}
    {% endfor %}
{% endblock qobject_body_header %}


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


{% if private_functions or members %}
private:
    {% for method in private_functions %}

        {% include "class_method_declaration_apidox_cpp.txt" %}
        {% include "class_method_declaration_cpp.txt" %}

    {% endfor %}
    {% for property in members %}

    {{property.type}} m_{{property.name}};
    {% endfor %}
{% endif %}
{% endblock class_body %}
