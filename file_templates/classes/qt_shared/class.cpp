{% extends "cpp_implementation.cpp" %}

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

{{ member.type }} {{ member.name }}() const
{
    return d->{{ member.name }};
}


void set{{ member.name|capfirst }}({{ member.type }} {{ member.name }})
{
    d->{{ member.name }} = {{ member.name }};
}

{% endfor %}

{% endblock function_definitions %}