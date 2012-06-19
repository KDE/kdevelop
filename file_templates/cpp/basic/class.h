{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H

class {{ name }}{% if is_inherited %} :{% for super in direct_inheritance_list %} public {{ super.identifier }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
public:
    {% for declaration in declarations %}
    {% with declaration.internal_declarations as arguments %}
        {% if declaration.is_static %}static {% endif %}{% if declaration.is_virtual %}virtual {% endif %}{% if declaration.type %}{{ declaration.type }} {% endif %}{{ declaration.identifier }}({% include "arguments_types_names.txt" %});
    {% endwith %}
    {% endfor %}
};

#endif // {{ name|upper }}_H