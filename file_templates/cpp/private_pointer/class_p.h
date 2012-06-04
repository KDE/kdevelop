/*

 {{ license }}
 */

#ifndef {{ name|upper }}_PRIVATE_H
#define {{ name|upper }}_PRIVATE_H

class {{ name }}Private
{
public:
    {{ name }}Private({{ name }}* q);
    virtual ~{{ name }}Private();
    
    {% for declaration in private_functions %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for declaration in private_members %}
        {% include "declaration.txt" %}
    {% endfor %}
    
private:
    class {{ name }}* const q;
};

#endif // {{ name|upper }}_PRIVATE_H