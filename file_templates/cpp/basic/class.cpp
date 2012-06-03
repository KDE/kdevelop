/*

  {{ license }}
 */

#include "{{ name }}.h"

{% for declaration in declarations %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}({% for argument in declaration.internal_declarations %}{{ argument.type }} {{ argument.identifier }}{% if not forloop.last %}, {% endif %}{% endfor %})
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endfor %}