{% load kdev_filters %}
{% block license_header %}
/*

 {{ license|lines_prepend:" * " }}
 */
{% endblock license_header %}


{% block include_guard_open %}
#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}
{% endblock include_guard_open %}


{% block includes %}
{% for included_file in included_files %}
#include {{ included_file }}
{% endfor %}
{% endblock includes %}


{% block namespaces_open %}
{% include "namespace_open_cpp.txt" %}
{% endblock namespaces_open %}


{% block forward_declarations %}
{% endblock forward_declarations %}


{% block class_declaration_open %}
class {{ name }}{% if base_classes %} :{% for base in base_classes %} {{ base.inheritanceMode }} {{ base.baseType }}{% if not forloop.last %},{% endif %}{% endfor %}{% endif %}
{
{% endblock class_declaration_open %}

{% block class_body %}
{% if public_functions %}
public:
    {% for method in public_functions %}
    {% include "method_declaration_cpp.txt" %}
    {% endfor %}


{% endif %}
{% if protected_functions %}
protected:
    {% for method in protected_functions %}
    {% include "method_declaration_cpp.txt" %}
    {% endfor %}


{% endif %}
{% if private_functions %}
private:
    {% for method in private_functions %}
    {% include "method_declaration_cpp.txt" %}
    {% endfor %}


{% endif %}
{% endblock class_body %}

{% block class_bottom %}
{% endblock %}

{% block class_declaration_close %}
};
{% endblock %}

{% block outside_class %}
{% endblock %}

{% block namespaces_close %}
{% include "namespace_close_cpp.txt" %}
{% endblock namespaces_close %}

{% block outside_namespace %}
{% endblock %}


{% block include_guard_close %}
#endif // {% include "include_guard_cpp.txt" %}
{% endblock include_guard_close %}