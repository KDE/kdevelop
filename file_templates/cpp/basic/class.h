/*

 {{ license }}
 */

#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H

class {{ name }}
{
public:
    {% for declaration in declarations %}
        {% if declaration.is_static %}static {% endif %}{% if declaration.is_virtual %}virtual {% endif %}{% if declaration.type %}{{ declaration.type }} {% endif %}{{ declaration.identifier }}({% for argument in declaration.internal_declarations %}{{ argument.type }} {{ argument.identifier }}{% if not forloop.last %}, {% endif %}{% endfor %});
    {% endfor %}
};

#endif // {{ name|upper }}_H