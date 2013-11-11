{% load kdev_filters %}
{% block license_header %}
{% if license %}
#

 {{ license|lines_prepend:"# " }}
#
{% endif %}
{% endblock license_header %}


import unittest


class {{ name }}(unittest.TestCase):
    def setUp(self):
        # Called before the first testfunction is executed
        pass


    def tearDown(self):
        # Called after the last testfunction was executed
        pass


    {% for case in testCases %}
    def test_{{ case }}(self):
        pass


    {% endfor %}


if __name__ == "__main__":
    unittest.main()