{% load kdev_filters %}
#

 {{ license|lines_prepend:"# " }}
#


import unittest


class {{ name }}(unittest.TestCase):
    def setUp(self):
        # Called before the first testfunction is executed
        pass


    def tearDown(self):
        # Called after the last testfunction was executed
        pass


    {% for case in testCases %}
    def test_{{ case.name }}(self):
        pass


    {% endfor %}


if __name__ == "__main__":
    unittest.main()