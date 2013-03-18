{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */


#include "{{ output_file_header }}"


{% for method in functions %}
{% with method.arguments as arguments %}

{{ method.returnType|default:"void" }} {{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %})
{
    
    
}

{% endwith %}
{% endfor %}
