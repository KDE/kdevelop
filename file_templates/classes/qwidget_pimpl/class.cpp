{% extends "cpp_qobject_pimpl_implementation.cpp" %}


{% block public_constructor_body %}
{{ block.super }}
    d_ptr->ui.setupUi(this);
{% endblock public_constructor_body %}


{% block protected_constructor_body %}
{{ block.super }}
    d_ptr->ui.setupUi(this);
{% endblock protected_constructor_body %}
