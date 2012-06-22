{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}

{% include "namespace_open_cpp.txt" %}

class {{ name }}{% if base_classes %} :{% for base in base_classes %} {{ base.inheritanceMode }} {{ base.baseType }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
public:
    {% for method in methods %}
    {% include "method_declaration_cpp.txt" %}
    {% endfor %}
};

{% include "namespace_close_cpp.txt" %}

#endif // {% include "include_guard_cpp.txt" %}