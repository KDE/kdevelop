{% load kdev_filters %}
{% include "license_header_cpp.txt" %}

#include <gtest/gtest.h>

{% for case in testCases %}

TEST({{ name }}, {{ case }})
{


}

{% endfor %}
