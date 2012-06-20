/*

  {{ license|lines_prepend" * " }}
 */

#include "{{ output_file_header }}"
#include "{{ output_file_private_header }}"

{% include "namespace_use_cpp.txt" %}

{{ private_class_name }}::{{ private_class_name }}({{ name }}* q) : q(q)
{

}

{{ private_class_name }}::~{{ private_class_name }}()
{

}

{% with private_class_name as name %}
{% for method in private_methods %}
{% include "method_definition_cpp.txt" %}
{

}
{% endfor %}
{% endwith %}

{% for method in public_methods %}
{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}    : {{ private_member_name}}(new {{ private_class_name }}(this)){% endif %}
{
    {% if method.isDestructor %}
    delete {{ private_member_name }};
    {% endif %}
}
{% endfor %}

{% for method in protected_methods %}
{% if method.isConstructor %}    : {{ private_member_name}}(new {{ private_class_name }}(this)){% endif %}
{% include "method_definition_cpp.txt" %}
{
    {% if method.isDestructor %}
    delete d;
    {% endif %}
}
{% endfor %}
