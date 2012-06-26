{% load kdev_filters %}
{% block license %}
/*
 * 
 {{ license|*lines_prepend:" * " }}
 */
{% endblock license %}

{% block includes %}
#include "{{ output_file_header }}"
{% endblock includes %}

{% block namespaces_use %}
{% include "namespace_use_cpp.txt" %}
{% endblock namespaces_use %}

{% block method_definitions %}
{% for method in functions %}
{% with method.arguments as arguments %}

{% if method.type %}{{ method.type }} {% endif %}{{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %})
{
    
}
{% endwith %}
{% endfor %}

{% endblock method_definitions %}
{% block bottom %}{% endblock %}