{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ output_file_header }}"


void initTestCase()
{
    // Called before the first testfunction is executed
}


void cleanupTestCase()
{
    // Called after the last testfunction was executed
}


void init()
{
    // Called before each testfunction is executed
}


void cleanup()
{
    // Called after every testfunction
}


{% for case in testCases %}
{% if case.isDataDriven %}

void {{ case.name }}_data()
{

}

{% endif %}

void {{ case.name }}()
{


}

{% endfor %}

QTEST_MAIN({{ name }});
#include {{ output_file_header|cut:".h" }}.moc