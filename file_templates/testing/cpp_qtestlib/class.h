{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}


#include <QObject>


class {{ name }} : public QObject
{
    Q_OBJECT


private Q_SLOTS:
    {% if testCase_initAndCleanup %}
    void initTestCase();
    void cleanupTestCase();


    {% endif %}
    {% if test_initAndCleanup %}
    void init();
    void cleanup();


    {% endif %}
    {% for case in testCases %}
    void {% if test_prefixMethods %}test{{ case|upper_first }}{% else %}{{ case }}{% endif %}();
    {% endfor %}
};


#endif // {% include "include_guard_cpp.txt" %}
