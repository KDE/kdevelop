{% extends "cpp_header_onlyfunctions.h" %}

{% block class_bottom %}
private:
    class {{ private_class_name }}* const {{ private_member_name }};
{% endblock class_bottom %}
