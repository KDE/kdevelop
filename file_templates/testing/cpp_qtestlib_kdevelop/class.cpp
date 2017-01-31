{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#include "{{ output_file_header }}"


#include <tests/testcore.h>
#include <tests/autotestshell.h>


QTEST_{% if test_nogui %}GUILESS_{% endif %}MAIN({{ name }});


using namespace KDevelop;


void {{ name }}::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::{% if test_nogui %}NoUi{% else %}Default{% endif %});
}


void {{ name }}::cleanupTestCase()
{
    TestCore::shutdown();
}


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
