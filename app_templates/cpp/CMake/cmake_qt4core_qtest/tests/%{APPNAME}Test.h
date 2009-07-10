#ifndef %{APPNAME}TEST_H
#define %{APPNAME}TEST_H

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

#endif // %{APPNAME}TEST_H
