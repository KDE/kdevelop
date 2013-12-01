{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H


#include <QObject>


class {{ name }} : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();


    void init();
    void cleanup();


    {% for case in testCases %}
    void {{ case }}();
    {% endfor %}
};


#endif // {{ name|upper }}_H