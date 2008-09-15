#include <QtTest/QTest>

class SegFault : public QObject
{
Q_OBJECT
private slots:
    void command() {
        int* i = 0, *j;
        *j = *i;
    }
};

QTEST_MAIN(SegFault)
#include "segFault.moc"
