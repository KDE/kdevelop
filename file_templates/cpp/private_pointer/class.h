/*

 {{ license }}
 */

#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H

class {{ name }}{% if is_inherited %} :{% for super in direct_inheritance_list %} public {{ super.identifier }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
public:
    {% for declaration in public_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for declaration in public_members %}
        {% include "declaration.txt" %}
    {% endfor %}
    
protected:
    {% for declaration in protected_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for declaration in protected_members %}
        {% include "declaration.txt" %}
    {% endfor %}
    
private:
    class {{ private_class_name }}* const {{ private_member_name }};
};

#endif // {{ name|upper }}_H