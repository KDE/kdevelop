/*

 {{ license }}
 */

#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H

namespace Ui
{
    class {{ name }};
}

class {{ name }}{% if is_inherited %} :{% for super in direct_inheritance_list %} public {{ super.identifier }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
    Q_OBJECT
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
    {% for declaration in private_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for declaration in private_members %}
        {% include "declaration.txt" %}
    {% endfor %}
    
private:
    Ui::{{ name }}* const ui;
};

#endif // {{ name|upper }}_H