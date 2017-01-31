{% load kdev_filters %}
{% include "license_header_cpp.txt" %}

#include "{{ output_file_header }}"


#include <QtTest>


{{ qtest_main }}({{ name }});


{% if testCase_initAndCleanup %}
void {{name}}::initTestCase()
{
    // Called before the first testfunction is executed
}


void {{name}}::cleanupTestCase()
{
    // Called after the last testfunction was executed
}


{% endif %}
{% if test_initAndCleanup %}
void {{name}}::init()
{
    // Called before each testfunction is executed
}


void {{name}}::cleanup()
{
    // Called after every testfunction
}


{% endif %}
{% for case in testCases %}

void {{ name }}::{% if test_prefixMethods %}test{{ case|upper_first }}{% else %}{{ case }}{% endif %}()
{


}

{% endfor %}
