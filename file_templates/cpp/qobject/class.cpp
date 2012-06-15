{% load kdev_filters %}
/*

  {{ license }}
 */

#include "{{ name }}.h"

class {{ name }}Private
{
public:
    {% for property in properties %}
    {{ property.type }} {{ property.name }};
    {% endfor %}
    
    {% for declaration in private_functions %}
    {% include "declaration.txt" %}
    {% endfor %}
};

{% for declaration in private_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}Private::{{ declaration.identifier }}({% include "arguments_types_names.txt" %})
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in public_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}({% include "arguments_types_names.txt" %}){% if declaration.is_constructor %}: d_ptr(new {{ name }}Private){% endif %}
{
    {% if declaration.is_destructor %}
    delete d;{% endif %}{% if declaration.type %}return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in protected_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}({% include "arguments_types_names.txt" %}){% if declaration.is_constructor %} : d_ptr(new {{ name }}Private){% endif %}
{
    {% if declaration.is_destructor %}
    delete d;{% endif %}{% if declaration.type %}return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for property in properties %}
{{ property.type }} {{ property.name }}() const
{
    Q_D(const {{ name }});
    return d->{{ property.name }};
}

void set{{ property.name|upper_first }}({{ property.type }} {{ property.name }})
{
    Q_D({{ name }});
    d->{{ property.name }} = {{ property.name }};
}
{% endfor %}