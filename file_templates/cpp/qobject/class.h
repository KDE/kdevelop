{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H

#include <QObject>

class {{ name }}Private;

class {{ name }}{% if is_inherited %} :{% for super in direct_inheritance_list %} public {{ super.identifier }}{% if not forloop.last %},{% endif %}{% endfor %}{% else %} : public QObject{% endif %}
{
    // This object has {{ properties|length }} properties.
    Q_OBJECT
    {% for property in properties %}
    Q_PROPERTY({{ property.type }} {{ property.name }} READ {{ property.name }} WRITE set{{ property.name|upper_first }})
    {% endfor %}
public:
    {% for declaration in public_functions %}
        {% include "declaration.txt" %}
    {% endfor %}
    
    {% for property in properties %}
    {{ property.type }} {{ property.name }}() const;
    {% endfor %}
    
public Q_SLOTS:
    {% for property in properties %}
    void set{{ property.name|upper_first }}({{ property.type }} {{ property.name }});
    {% endfor %}
    
protected:
    {% for declaration in protected_functions %}
        {% include "declaration.txt" %}
    {% endfor %}
    
private:
    Q_DECLARE_PRIVATE({{ name }})
};

#endif // {{ name|upper }}_H