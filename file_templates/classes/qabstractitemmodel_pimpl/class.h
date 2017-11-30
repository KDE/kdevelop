{% extends "cpp_qobject_pimpl_header.h" %}


{% block qobject_default_include %}
#include <QAbstractItemModel>
{% endblock qobject_default_include %}


{% block qobject_baseclass %}QAbstractItemModel{% endblock qobject_baseclass %}


{% block qobject_body_header %}
{{ block.super }}
{% if add_roles_enum %}


public:
    enum {{ roles_enum_name }} {
        {{ first_role_name }} = Qt::UserRole
    };
{% endif %}
{% endblock qobject_body_header %}
