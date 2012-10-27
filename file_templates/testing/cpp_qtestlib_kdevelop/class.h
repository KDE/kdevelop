{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */


#ifndef {{ name|upper }}_H
#define {{ name|upper }}_H


#include <QObject>


class {{ name }} : public QObject
{
private slots:
    void initTestCase();
    void cleanupTestCase();


    {% for case in testCases %}
    void {{ case }}();
    {% endfor %}
};


#endif // {{ name|upper }}_H