#ifndef MONEYTEST_H
#define MONEYTEST_H

#include <QObject>

class MoneyTest : public QObject
{
Q_OBJECT
private slots:
    void create();
    void negative();
    void zero();
};

#endif // MONEYTEST_H
