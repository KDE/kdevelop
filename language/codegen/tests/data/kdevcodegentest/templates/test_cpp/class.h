{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */


#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H


class {{ name }}{% if base_classes %} :{% for base in base_classes %} {{ base.inheritanceMode }} {{ base.baseType }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
public:
{% for method in functions %}
{% with method.arguments as arguments %}
    {{ method.returnType|default:"void" }} {{ method.name }}({% include "arguments_types_names.txt" %});
{% endwith %}
{% endfor %}


{% for member in members %}
    {{ member.type }} {{ member.name }};
{% endfor %}

};


#endif // {{ name|upper }}_H
