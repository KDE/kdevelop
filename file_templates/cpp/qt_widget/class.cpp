/*

  {{ license }}
 */

#include "{{ name }}.h"
#include "ui_{{ name }}.h"

{% for declaration in functions %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}{% include "arguments.txt" %}{% if declaration.is_constructor %}: ui(new Ui::{{ name }}){% endif %}
{
    {% if declaration.is_constructor %}
    ui->setupUi(this);{% endif %}{% if declaration.is_destructor %}
    delete ui;{% endif %}{% if declaration.type %}return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}
