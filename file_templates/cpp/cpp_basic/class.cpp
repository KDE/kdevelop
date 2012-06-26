{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ output_file_header }}"

{% include "namespace_use_cpp.txt" %}

{% for method in functions %}
{% with method.arguments as arguments %}

{% if method.type %}{{ method.type }} {% endif %}{{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %})
{

}
{% endwith %}
{% endfor %}