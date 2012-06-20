{% load kdev_filters %}
"""

{{ license|lines_prepend:" " }}
"""

class {{ name }}{% if is_inherited %}({% for inh in direct_inheritance_list %}{{ inh.name }}{% if forloop.last %}, {% endif %}{% endfor %}){% endif %}:
    def __init__(self{% for property in properties %}, {{ property.name }}{% endfor %}):
        {% for inh in direct_inheritance_list %}
        {{ inh.name }}.__init__(self)
        {% endfor %}

        {% for property in properties %}
        self.{{ property.name }} = {{ property.name }}
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