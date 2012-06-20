/*

 {{ license|lines_prepend:" * " }}
 */

#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}

{% include "namespace_begin_cpp.txt" %}

{% include "class_declaration_cpp.txt" %}
{
public:
    {% for method in public_methods %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for member in public_members %}
    {{ member.type }} {{ member.name }};
    {% endfor %}

protected:
    {% for method in protected_methods %}
        {% include "declaration.txt" %}
    {% endfor %}

    {% for member in protected_members %}
    {{ member.type }} {{ member.name }};
    {% endfor %}

private:
    class {{ private_class_name }}* const {{ private_member_name }};
};

{% include "namespace_end_cpp.txt" %}

#endif // {% include "include_guard_cpp.txt" %}