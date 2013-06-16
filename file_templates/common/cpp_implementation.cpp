{% load kdev_filters %}
{% block license_header %}
/*

 {{ license|lines_prepend:" * " }}
 */
{% endblock license_header %}


{% block includes %}
#include "{{ output_file_header }}"
{% endblock includes %}


{% block namespaces_use %}
{% include "namespace_use_cpp.txt" %}
{% endblock namespaces_use %}


{% block extra_declarations %}
{% endblock extra_declarations %}


{% block extra_definitions %}
{% endblock extra_definitions %}


{% block function_definitions %}
{% for method in functions %}
{% include "method_definition_cpp.txt" %}
{


}


{% endfor %}

{% endblock function_definitions %}


{% block bottom %}
{% endblock %}