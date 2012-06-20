/*

 {{ license|lines_prepend:" * " }}
 */
{% with private_class_name as name %}

#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}

{% include "namespace_begin_cpp.txt" %}

class {{ private_class_name }}
{
public:
    {{ private_class_name }}({{ name }}* q);
    virtual ~{{ private_class_name }}();

    {% for method in private_methods %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for member in private_members %}
    {{ member.type }} {{ member.name }};
    {% endfor %}

private:
    class {{ name }}* const q;
};

{% include "namespace_end_cpp.txt" %}

#endif // {{ private_class_name|upper}}_H
{% endwith %}