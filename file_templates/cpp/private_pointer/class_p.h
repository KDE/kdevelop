/*

 {{ license }}
 */

#ifndef {{ private_class_name|upper }}_H
#define {{ private_class_name|upper }}_H

class {{ private_class_name }}
{
public:
    {{ private_class_name }}({{ name }}* q);
    virtual ~{{ private_class_name }}();
    
    {% for declaration in private_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for declaration in private_members %}
        {% include "declaration.txt" %}
    {% endfor %}
    
private:
    class {{ name }}* const q;
};

#endif // {{ private_class_name|upper}}_H