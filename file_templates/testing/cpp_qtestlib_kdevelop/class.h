{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#ifndef {% include "include_guard_cpp.txt" %}
#define {% include "include_guard_cpp.txt" %}


#include <QObject>


class {{ name }} : public QObject
{
    Q_OBJECT


private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();


    {% for case in testCases %}
    void {% if test_prefixMethods %}test{{ case|upper_first }}{% else %}{{ case }}{% endif %}();
    {% endfor %}
};


#endif // {% include "include_guard_cpp.txt" %}
