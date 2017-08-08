{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ output_file_header }}"


void {{ name }}::initTestCase()
{
    // Called before the first testfunction is executed
}


void {{ name }}::cleanupTestCase()
{
    // Called after the last testfunction was executed
}


void {{ name }}::init()
{
    // Called before each testfunction is executed
}


void {{ name }}::cleanup()
{
    // Called after every testfunction
}


{% for case in testCases %}

void {{ name }}::{{ case }}()
{


}

{% endfor %}

QTEST_MAIN({{ name }});
#include "{{ output_file_header|cut:".h" }}.moc"