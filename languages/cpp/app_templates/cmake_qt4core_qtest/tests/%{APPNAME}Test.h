#ifndef %{PROJECTDIRNAME}_%{APPNAME}TEST_H
#define %{PROJECTDIRNAME}_%{APPNAME}TEST_H

#include <QtCore/QObject>

class %{APPNAME}Test : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void someTest();
};

#endif // %{PROJECTDIRNAME}_%{APPNAME}TEST_H
