{% load kdev_filters %}
{% block license_header %}
{% if license %}
#

{{ license|lines_prepend:"# " }}
#
{% endif %}
{% endblock license_header %}


class {{ name }}{% if base_classes %}({% for inh in base_classes %}{{ inh.baseType }}{% if not forloop.last %}, {% endif %}{% endfor %}){% endif %}:
    def __init__(self{% for member in members %}, {{ member.name }}{% endfor %}):
        {% for inh in base_classes %}
        {{ inh.baseType }}.__init__(self)
        {% endfor %}


        {% for member in members %}
        self.{{ member.name }} = {{ member.name }}
        {% empty %}
        pass
        {% endfor %}


    {% for method in methods %}

    {% if method.isStatic %}
    @staticmethod
    def {{ method.name }}({% include "arguments_names.txt" %}):
    {% else %}{% if method.arguments %}
    def {{ method.name }}(self{% for arg in method.arguments %}, {{ arg.name }}{% endfor %}):
    {% else %}
    def {{ method.name }}(self):
    {% endif %}{% endif %}
        pass

    {% endfor %}