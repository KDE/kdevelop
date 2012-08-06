{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */


#include "{{ output_file_header }}"
#include <qtest_kde.h>


#include <tests/testcore.h>
#include <tests/autotestshell.h>


using namespace KDevelop;


void initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUI);
}


void cleanupTestCase()
{
    TestCore::shutdown();
}


{% for case in testCases %}

void {{ case }}()
{


}

{% endfor %}

QTEST_KDEMAIN({{ name }}, NoGUI);
#include {{ output_file_header|cut:".h" }}.moc