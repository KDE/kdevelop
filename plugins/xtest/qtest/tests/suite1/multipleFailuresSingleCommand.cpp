#include <QtTest/QTest>

class MultipleFailuresCase : public QObject
{
Q_OBJECT
private slots:
    void command() {
        fail1();
        fail2();
    }
private:
    void fail1() {
        QVERIFY2(0, "fail1");
    }
    void fail2() {
        QVERIFY2(0, "fail2");
    }
};

QTEST_MAIN(MultipleFailuresCase)
#include "multipleFailuresSingleCommand.moc"
