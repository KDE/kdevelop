#include <QtTest/QTest>
#include <iostream>

class SegFault : public QObject
{
Q_OBJECT
private slots:
    void command() {
        int* i = 0;
	std::cout << *i;
    }
};

QTEST_MAIN(SegFault)
#include "segFault.moc"
