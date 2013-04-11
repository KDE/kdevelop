{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */


#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H


#include <QtTest/QtTest>


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