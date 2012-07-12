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
class {{ name }} : public QObject
{
{% endif %}
{% endblock class_declaration_open %}

{% block class_body %}
    Q_OBJECT
    {% for property in properties %}
    Q_PROPERTY({{ property.type }} {{ property.name }} READ {{ property.name }} WRITE set{{ property.name|upper_first }})
    {% endfor %}


{% if public_functions or members %}
public:
{% endif %}
    {% for method in public_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for property in members %}
    {{ property.type }} {{ property.name }}() const;
    {% endfor %}


{% if members %}
public Q_SLOTS:
    {% for property in members %}
    void set{{ property.name|upper_first }}({{ property.type }} {{ property.name }});
    {% endfor %}
{% endif %}


{% if protected_functions %}
protected:
    {% for method in protected_functions %}
        {% include "declaration.txt" %}
    {% endfor %}
{% endif %}


private:
    Q_DECLARE_PRIVATE({{ name }})
{% endblock class_body %}