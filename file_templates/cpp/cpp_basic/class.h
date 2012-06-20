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
    {% with method.arguments as arguments %}
        {% if declaration.isStatic %}static {% endif %}{% if declaration.isVirtual %}virtual {% endif %}{% if method.returnType %}{{ method.returnType }} {% endif %}{{ method.name }}({% include "arguments_types_names.txt" %});
    {% endwith %}
    {% endfor %}
};

{% include "namespace_close_cpp.txt" %}

#endif // {% include "include_guard_cpp.txt" %}