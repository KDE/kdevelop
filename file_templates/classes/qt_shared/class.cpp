{% extends "cpp_implementation.cpp" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
#include <QSharedData>
{% endblock includes %}

{% block extra_declarations %}
class {% if namespaces %}{{ namespaces|join:"::" }}::{% endif %}{{ name }}Data : public QSharedData
{
public:
{% for member in members %}
{{ member.type }} {{ member.name }};
{% endfor %}
};

{% endblock extra_declarations %}

{% block function_definitions %}

{% for member in members %}

{{ member.type }} {{ name }}::{{ member.name }}() const
{
    return d->{{ member.name }};
}


void {{ name }}::set{{ member.name|capfirst }}({{ member.type|arg_type }} {{ member.name }})
{
    d->{{ member.name }} = {{ member.name }};
}

{% endfor %}

{% endblock function_definitions %}
