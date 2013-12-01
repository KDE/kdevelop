{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#include "{{ output_file_header }}"
#include "{{ output_file_privateheader }}"


{% include "namespace_use_cpp.txt" %}


{{ private_class_name }}::{{ private_class_name }}({{ name }}* q) : q(q)
{

}

{{ private_class_name }}::~{{ private_class_name }}()
{

}

{% for method in private_functions %}

{% with method.arguments as arguments %}
{{ method.returnType|default:"void" }} {{ name }}::{{ method.name }}({% include "arguments_types_names.txt" %}){% if method.isConst %} const{% endif %}
{% endwith %}
{


}

{% endfor %}

{% for method in public_functions %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %} : {{ private_member_name}}(new {{ private_class_name }}(this)){% endif %}
{
    {% if method.isDestructor %}
    delete {{ private_member_name }};
    {% endif %}


}

{% endfor %}

{% for method in protected_functions %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %} : {{ private_member_name}}(new {{ private_class_name }}(this)){% endif %}
{
    {% if method.isDestructor %}
    delete d;
    {% endif %}


}

{% endfor %}
