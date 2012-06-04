/*

  {{ license }}
 */

#include "{{ name }}.h"
#include "{{ name }}_p.h"

{{ name }}Private::{{ name }}Private({{ name }}* q) : q(q)
{
    
}

{{ name }}Private::~{{ name }}Private()
{
    
}

{% for declaration in private_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}Private::{{ declaration.identifier }}{% include "arguments.txt" %}
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in public_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}
{% if declaration.is_constructor %}: d(new {{ name }}Private(this)){% endif %}
{
    {% if declaration.is_destructor %}
    delete d;
    {% endif %}
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in protected_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}
{% if declaration.is_constructor %}: d(new {{ name }}Private(this)){% endif %}
{
    {% if declaration.is_destructor %}
    delete d;
    {% endif %}
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}