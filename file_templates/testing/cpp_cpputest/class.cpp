{% load kdev_filters %}
{% include "license_header_cpp.txt" %}

#include <CppUTest/TestHarness.h>


TEST_GROUP({{ name }}) {


};

{% for case in testCases %}

TEST({{ name }}, {{ case }})
{


}

{% endfor %}
