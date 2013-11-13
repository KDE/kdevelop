{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#include "{{ output_file_header }}"


#include <qtest_kde.h>


#include <tests/testcore.h>
#include <tests/autotestshell.h>


QTEST_KDEMAIN({{ name }}, NoGUI);


using namespace KDevelop;


void {{ name }}::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}


void {{ name }}::cleanupTestCase()
{
    TestCore::shutdown();
}


{% for case in testCases %}

void {{ name }}::{{ case }}()
{


}

{% endfor %}

#include "{{ output_file_header|cut:".h"|cut:".hpp" }}.moc"
