{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ name }}.h"

{% for declaration in declarations %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}