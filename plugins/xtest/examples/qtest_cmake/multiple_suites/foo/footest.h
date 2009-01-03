#ifndef QTEST_FOOTEST_H_INCLUDED
#define QTEST_FOOTEST_H_INCLUDED

#include <QtCore/QObject>

class FooTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void someCmd();
};

#endif // QTEST_FOOTEST_H_INCLUDED
