{% extends "cpp_implementation.cpp" %}


{% block function_definitions %}

{% for method in public_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
{% block public_constructor_init_list %}
{% endblock public_constructor_init_list %}
{% endif %}
{
{% block public_method_body %}
    {% if method.isConstructor %}
{% block public_constructor_body %}
{% endblock public_constructor_body %}
    {% elif method.isDestructor %}
{% block public_destructor_body %}
{% endblock public_destructor_body %}
    {% elif method.type %}
    return {{ method.default_return_value }};
    {% endif %}
{% endblock public_method_body %}
}

{% endwith %}
{% endfor %}

{% for property in members %}


{% include "class_property_getter_definition_cpp.txt" %}
{
    return m_{{ property.name }};
}


{% include "class_property_setter_definition_cpp.txt" %}
{
    if (m_{{ property.name }} == {{ property.name }}) {
        return;
    }


    m_{{ property.name }} = {{ property.name }};
    emit {{ property.name }}Changed(m_{{ property.name }});
}

{% endfor %}

{% for method in protected_functions %}
{% with method.arguments as arguments %}

{% include "method_definition_cpp.txt" %}
{% if method.isConstructor %}
{% block protected_constructor_init_list %}
{% endblock protected_constructor_init_list %}
{% endif %}
{
{% block protected_method_body %}
   {% if method.isConstructor %}
{% block protected_constructor_body %}
{% endblock protected_constructor_body %}
    {% elif method.isDestructor %}
{% block protected_destructor_body %}
{% endblock protected_destructor_body %}
    {% elif method.type %}
    return {{ method.default_return_value }};
    {% endif %}
{% endblock protected_method_body %}
}

{% endwith %}
{% endfor %}

{% for method in private_functions %}

{% include "method_definition_cpp.txt" %}
{
    {% if method.type %}
    return {{ method.default_return_value }};
    {% endif %}
}

{% endfor %}
{% endblock function_definitions %}
