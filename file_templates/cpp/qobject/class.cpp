{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ output_file_header }}"

class {{ name }}Private
{
public:
    {% for property in properties %}
    {{ property.type }} {{ property.name }};
    {% endfor %}

    {% for method in private_methods %}
    {% include "method_declaration.txt" %}
    {% endfor %}
};

{% for method in private_methods %}
{% with method.arguments as arguments %}

{% if method.type %}{{ method.type }} {% endif %}{{ name }}Private::{{ method.name }}({% include "arguments_types_names.txt" %})
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for method in public_methods %}
{% with method.arguments as arguments %}

{% if method.type %}{{ method.type }} {% endif %}{{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %}){% if method.is_constructor %}: d_ptr(new {{ name }}Private){% endif %}
{
    {% if method.isDestructor %}
    delete d;{% endif %}{% if method.type %}return {{ method.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}

{% for method in protected_methods %}
{% with method.arguments as arguments %}

{% if method.type %}{{ method.type }} {% endif %}{{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %}){% if method.is_constructor %} : d_ptr(new {{ name }}Private){% endif %}
{
    {% if method.isDestructor %}
    delete d;{% endif %}{% if method.type %}return {{ method.default_return_value }};
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