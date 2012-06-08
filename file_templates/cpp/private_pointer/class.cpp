/*

  {{ license }}
 */

#include "{{ name }}.h"
#include "{{ name }}_p.h"

{{ private_class_name }}::{{ private_class_name }}({{ name }}* q) : q(q)
{
    
}

{{ private_class_name }}::~{{ private_class_name }}()
{
    
}

{% for declaration in private_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ private_class_name }}::{{ declaration.identifier }}{% include "arguments.txt" %}
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in public_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}{% if declaration.is_constructor %}: {{ private_member_name }}(new {{ private_class_name }}(this)){% endif %}
{
    {% if declaration.is_destructor %}
    delete {{ private_member_name }};
    {% endif %}
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for declaration in protected_functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}{% if declaration.is_constructor %} : {{ private_member_name }}(new {{ private_class_name }}(this)){% endif %}
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