{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}

#include <QObject>

{% include "namespace_open_cpp.txt" %}

class {{ name }}Private;

class {{ name }}{% if base_classes %} :{% for base in base_classes %} {{ base.inheritanceMode }} {{ base.baseType }}{% if not forloop.last %},{% endif %}{% endfor %}{% else %} : public QObject{% endif %}
{
    Q_OBJECT
    {% for property in properties %}
    Q_PROPERTY({{ property.type }} {{ property.name }} READ {{ property.name }} WRITE set{{ property.name|upper_first }})
    {% endfor %}
public:
    {% for method in public_methods %}
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
    {% for method in protected_methods %}
        {% include "declaration.txt" %}
    {% endfor %}

private:
    Q_DECLARE_PRIVATE({{ name }})
};

#endif // {% include "include_guard_cpp.txt" %}