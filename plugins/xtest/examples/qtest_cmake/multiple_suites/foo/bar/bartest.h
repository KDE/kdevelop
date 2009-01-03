#ifndef QTEST_FOO_BARTEST_H_INCLUDED
#define QTEST_FOO_BARTEST_H_INCLUDED

#include <QtCore/QObject>

class BarTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void someCmd();
};

#endif // QTEST_BARTEST_H_INCLUDED
