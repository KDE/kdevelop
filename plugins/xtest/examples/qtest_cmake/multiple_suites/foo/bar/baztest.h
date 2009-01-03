#ifndef QTEST_BAZTEST_H_INCLUDED
#define QTEST_BAZTEST_H_INCLUDED

#include <QtCore/QObject>

class BazTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void someCmd();
};

#endif // QTEST_BAZTEST_H_INCLUDED
